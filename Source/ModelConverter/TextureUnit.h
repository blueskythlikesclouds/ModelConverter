#pragma once

struct SampleChunk;

struct TextureUnit
{
    std::string name;
    uint8_t index;

    void write(SampleChunk& out) const;
};
