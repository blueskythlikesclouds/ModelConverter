#include "TextureUnit.h"

#include "SampleChunkWriter.h"

void TextureUnit::write(SampleChunkWriter& writer) const
{
    writer.writeOffset(1, [&]
    {
        writer.write(name);
    });
    writer.write<uint8_t>(index);
}
