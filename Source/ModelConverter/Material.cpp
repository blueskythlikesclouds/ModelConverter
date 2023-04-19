#include "Material.h"

#include "Config.h"
#include "Parameter.h"
#include "SampleChunkNode.h"
#include "SampleChunkWriter.h"
#include "Texture.h"

Material::Material()
    : shader("Common_d")
    , layer("solid")
    , alphaThreshold(128)
    , doubleSided(false)
    , additive(false)
{
}

Material::~Material() = default;

template<typename T>
static void writeParameters(SampleChunkWriter& writer, const std::vector<T>& parameters)
{
    writer.writeOffset(4, [&]
    {
        for (auto& parameter : parameters)
        {
            writer.writeOffset(4, [&]
            {
                parameter.write(writer);
            });
        }
    });
}

void Material::write(SampleChunkWriter& writer) const
{
    writer.writeOffset(1, [&]
    {
        writer.write(shader); 
    });
    writer.writeOffset(1, [&]
    {
        writer.write(shader); 
    });
    writer.writeOffset(4, [&]
    {
        for (auto& texture : textures)
        {
            writer.writeOffset(1, [&]
            {
                writer.write(texture.name);
            });
        }
    });
    writer.writeOffset(4, [&]
    {
        for (auto& texture : textures)
        {
            writer.writeOffset(4, [&]
            {
                texture.write(writer);
            });
        }
    });
    writer.write<uint8_t>(alphaThreshold);
    writer.write(static_cast<uint8_t>(doubleSided));
    writer.write(static_cast<uint8_t>(additive));
    writer.write<uint8_t>(0);
    writer.write(static_cast<uint8_t>(float4Parameters.size()));
    writer.write(static_cast<uint8_t>(int4Parameters.size()));
    writer.write(static_cast<uint8_t>(boolParameters.size()));
    writer.write(static_cast<uint8_t>(textures.size()));
    writeParameters(writer, float4Parameters);
    writeParameters(writer, int4Parameters);
    writeParameters(writer, boolParameters);
}

void Material::save(const char* path, Config config) const
{
    if (config & CONFIG_FLAG_V2_SAMPLE_CHUNK)
    {
        SampleChunkNode material("Material", 1);

        if (!scaParameters.empty())
        {
            auto& scaParam = material.children.emplace_back("SCAParam", 1);

            for (const auto& [name, value] : scaParameters)
                scaParam.children.emplace_back(name, value);
        }

        material.children.emplace_back("Contexts", 3, [&](SampleChunkWriter& writer)
        {
            write(writer);
        });

        SampleChunkWriter::write(material).save(path);
    }
    else
    {
        SampleChunkWriter::write(3, [&](SampleChunkWriter& writer)
        {
            write(writer);
        }).save(path);
    }
}
