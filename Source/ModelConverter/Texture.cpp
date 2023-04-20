#include "Texture.h"

#include "SampleChunkWriter.h"

Texture::Texture()
    : texCoordIndex()
    , addressU()
    , addressV()
{
}

void Texture::write(SampleChunkWriter& writer) const
{
    writer.writeOffset(1, [&]
    {
        writer.write(pictureName);
    });
    writer.write<uint8_t>(texCoordIndex);
    writer.write(static_cast<uint8_t>(addressU));
    writer.write(static_cast<uint8_t>(addressV));
    writer.write<uint8_t>(0);
    writer.writeOffset(1, [&]
    {
        writer.write(type);
    });
}

bool Texture::save(const char* path) const
{
    return SampleChunkWriter::write(1, [&](SampleChunkWriter& writer)
    {
        write(writer);
    }).save(path);
}
