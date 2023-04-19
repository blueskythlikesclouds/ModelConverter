#include "Texture.h"

#include "SampleChunk.h"

Texture::Texture()
    : texCoordIndex()
    , addressU()
    , addressV()
{
}

void Texture::write(SampleChunk& out) const
{
    out.writeOffset(1, [&]
    {
        out.write(pictureName);
    });
    out.write<uint8_t>(texCoordIndex);
    out.write(static_cast<uint8_t>(addressU));
    out.write(static_cast<uint8_t>(addressV));
    out.write<uint8_t>(0);
    out.writeOffset(1, [&]
    {
        out.write(type);
    });
}
