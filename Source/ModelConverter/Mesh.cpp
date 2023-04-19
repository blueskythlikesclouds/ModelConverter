#include "Mesh.h"

#include "SampleChunk.h"
#include "TextureUnit.h"
#include "VertexElement.h"

Mesh::Mesh() = default;
Mesh::~Mesh() = default;

void Mesh::write(SampleChunk& out) const
{
    size_t vertexSize = 0;

    for (const auto& vertexElement : vertexElements)
        vertexSize = std::max(vertexSize, vertexElement.getNextOffset());

    out.writeOffset(1, [&]
    {
        out.write(materialName);
    });

    out.write(static_cast<uint32_t>(faceIndices.size()));
    out.writeOffset(2, [&]
    {
        for (const auto index : faceIndices)
            out.write<uint16_t>(index);
    });

    out.write(static_cast<uint32_t>(vertexStreams[0][0].size()));
    out.write(static_cast<uint32_t>(vertexSize));
    out.writeOffset(4, [&, vertexSize]
    {
        const size_t offset = out.position;

        for (size_t i = 0; i < vertexStreams[0][0].size(); i++)
        {
            for (const auto& vertexElement : vertexElements)
            {
                out.position = offset + (i * vertexSize) + vertexElement.offset;
                vertexElement.write(out, vertexStreams[static_cast<size_t>(vertexElement.type)][vertexElement.index][i]);
            }
        }

        out.position = offset + vertexSize * vertexStreams[0][0].size();
    });
    out.writeOffset(4, [&]
    {
        for (auto& vertexElement : vertexElements)
            vertexElement.write(out);

        VertexElement().write(out);
    });

    out.write(static_cast<uint32_t>(nodeIndices.size()));
    out.writeOffset(out.dataVersion >= 6 ? 2 : 1, [&]
    {
        if (out.dataVersion >= 6)
        {
            for (const auto index : nodeIndices)
                out.write<uint16_t>(index);
        }
        else
        {
            for (const auto index : nodeIndices)
                out.write(static_cast<uint8_t>(index));
        }
    });

    out.write(static_cast<uint32_t>(textureUnits.size()));
    out.writeOffset(4, [&]
    {
        for (auto& unit : textureUnits)
        {
            out.writeOffset(4, [&]
            {
                unit.write(out);
            });
        }
    });
}
