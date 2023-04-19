#include "Material.h"

#include "Parameter.h"
#include "SampleChunk.h"
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
static void writeParameters(SampleChunk& out, const std::vector<T>& parameters)
{
    out.writeOffset(4, [&]
    {
        for (auto& parameter : parameters)
        {
            out.writeOffset(4, [&]
            {
                parameter.write(out);
            });
        }
    });
}

void Material::write(SampleChunk& out) const
{
    out.writeOffset(1, [&]
    {
        out.write(shader); 
    });
    out.writeOffset(1, [&]
    {
        out.write(shader); 
    });
    out.writeOffset(4, [&]
    {
        for (auto& texture : textures)
        {
            out.writeOffset(1, [&]
            {
                out.write(texture.name);
            });
        }
    });
    out.writeOffset(4, [&]
    {
        for (auto& texture : textures)
        {
            out.writeOffset(4, [&]
            {
                texture.write(out);
            });
        }
    });
    out.write<uint8_t>(alphaThreshold);
    out.write(static_cast<uint8_t>(doubleSided));
    out.write(static_cast<uint8_t>(additive));
    out.write<uint8_t>(0);
    out.write(static_cast<uint8_t>(float4Parameters.size()));
    out.write(static_cast<uint8_t>(int4Parameters.size()));
    out.write(static_cast<uint8_t>(boolParameters.size()));
    out.write(static_cast<uint8_t>(textures.size()));
    writeParameters(out, float4Parameters);
    writeParameters(out, int4Parameters);
    writeParameters(out, boolParameters);
}

void Material::save(const char* path) const
{
    SampleChunk out;
    out.begin(3);
    write(out);
    out.end();
    out.save(path);
}
