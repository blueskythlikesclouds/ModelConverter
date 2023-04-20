#include "Mesh.h"

#include "SampleChunkWriter.h"
#include "TextureUnit.h"
#include "VertexElement.h"

Mesh::Mesh() = default;
Mesh::~Mesh() = default;

void Mesh::write(SampleChunkWriter& writer, uint32_t dataVersion) const
{
    size_t vertexSize = 0;

    for (const auto& vertexElement : vertexElements)
        vertexSize = std::max(vertexSize, vertexElement.getNextOffset());

    writer.writeOffset(1, 1, [&]
    {
        writer.write(materialName);
    });

    writer.write(static_cast<uint32_t>(faceIndices.size()));
    writer.writeOffset(2, [&]
    {
        for (const auto index : faceIndices)
            writer.write(static_cast<uint16_t>(index));
    });

    writer.write(static_cast<uint32_t>(vertexStreams[0][0].size()));
    writer.write(static_cast<uint32_t>(vertexSize));
    writer.writeOffset(4, [&, vertexSize]
    {
        const size_t offset = writer.currentOffset;

        for (size_t i = 0; i < vertexStreams[0][0].size(); i++)
        {
            for (const auto& vertexElement : vertexElements)
            {
                writer.currentOffset = offset + (i * vertexSize) + vertexElement.offset;
                vertexElement.write(writer, vertexStreams[static_cast<size_t>(vertexElement.type)][vertexElement.index][i]);
            }
        }

        writer.currentOffset = offset + vertexSize * vertexStreams[0][0].size();
    });
    writer.writeOffset(4, [&]
    {
        for (auto& vertexElement : vertexElements)
            vertexElement.write(writer);

        VertexElement().write(writer);
    });

    writer.write(static_cast<uint32_t>(nodeIndices.size()));
    writer.writeOffset(dataVersion >= 6 ? 2 : 1, [&, dataVersion]
    {
        if (dataVersion >= 6)
        {
            for (const auto index : nodeIndices)
                writer.write<uint16_t>(index);
        }
        else
        {
            for (const auto index : nodeIndices)
                writer.write(static_cast<uint8_t>(index));
        }
    });

    writer.write(static_cast<uint32_t>(textureUnits.size()));
    writer.writeOffset(4, [&]
    {
        for (auto& unit : textureUnits)
        {
            writer.writeOffset(4, [&]
            {
                unit.write(writer);
            });
        }
    });
}
