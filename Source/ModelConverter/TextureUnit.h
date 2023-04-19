#pragma once

struct SampleChunkWriter;

struct TextureUnit
{
    std::string name;
    uint8_t index;

    void write(SampleChunkWriter& writer) const;
};
