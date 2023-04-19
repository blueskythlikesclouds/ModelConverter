#include "TextureUnit.h"

#include "SampleChunk.h"

void TextureUnit::write(SampleChunk& out) const
{
    out.writeOffset(1, [&]
    {
        out.write(name);
    });
    out.write<uint8_t>(index);
}
