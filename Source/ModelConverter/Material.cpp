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

void Material::write(SampleChunkWriter& writer, uint32_t dataVersion) const
{
    writer.writeOffset(1, [&]
    {
        writer.write(shader); 
    });
    writer.writeOffset(1, [&]
    {
        writer.write(shader); 
    });
    if (dataVersion >= 3)
    {
        writer.writeOffset(1, 4, [&]
        {
            for (auto& texture : textures)
            {
                writer.writeOffset(1, [&]
                {
                    writer.write(texture.name);
                });
            }
        });
        writer.writeOffset(1, 4, [&]
        {
            for (auto& texture : textures)
            {
                writer.writeOffset(4, [&]
                {
                    texture.write(writer);
                });
            }
        });
    }
    else
    {
        writer.writeOffset(1, [&]
        {
            writer.write(name);
        });
        writer.write<uint32_t>(0);
    }
    writer.write<uint8_t>(alphaThreshold);
    writer.write(static_cast<uint8_t>(doubleSided));
    writer.write(static_cast<uint8_t>(additive));
    writer.write<uint8_t>(0);
    writer.write(static_cast<uint8_t>(float4Parameters.size()));
    writer.write(static_cast<uint8_t>(int4Parameters.size()));
    writer.write(static_cast<uint8_t>(boolParameters.size()));

    if (dataVersion >= 3)
        writer.write(static_cast<uint8_t>(textures.size()));
    else
        writer.write<uint8_t>(0);

    writeParameters(writer, float4Parameters);
    writeParameters(writer, int4Parameters);
    writeParameters(writer, boolParameters);
}

bool Material::save(const char* path, Config config) const
{
    if (config & CONFIG_FLAG_V1_MATERIAL)
    {
        bool result = SampleChunkWriter::write(1, [&](SampleChunkWriter& writer)
        {
            write(writer, 1);
        }).save(path);

        if (!result)
            return false;

        std::string dir = path;
        dir.erase(dir.find_last_of("\\/") + 1);

        std::string texset = dir;
        texset += name;
        texset += ".texset";

        result = SampleChunkWriter::write(0, [&](SampleChunkWriter& writer)
        {
            writer.write(static_cast<uint32_t>(textures.size()));
            writer.writeOffset(4, [&]
            {
               for (const auto& texture : textures)
               {
                   writer.writeOffset(1, [&]
                   {
                       writer.write(texture.name);
                   });
               }
            });
        }).save(texset.c_str());

        if (!result)
            return false;

        for (const auto& texture : textures)
        {
            std::string tex = dir;
            tex += texture.name;
            tex += ".texture";

            if (!texture.save(tex.c_str()))
                return false;
        }

        return true;
    }
    else
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
                write(writer, 3);
            });

            return SampleChunkWriter::write(material).save(path);
        }
        else
        {
            return SampleChunkWriter::write(3, [&](SampleChunkWriter& writer)
            {
                write(writer, 3);
            }).save(path);
        }
    }
}
