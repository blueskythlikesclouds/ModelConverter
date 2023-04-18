#include "TextureUnit.h"

#include "SampleChunk.h"

void TextureUnit::write(SampleChunk& out) const
{
    out.writeOffset(4, [&]
    {
        out.write(name);
    });
    out.write<uint8_t>(index);
}
