#pragma once

struct SampleChunk;

enum class AddressMode
{
    Repeat,
    Mirror,
    Clamp,
    MirrorOnce,
    Border
};

struct Texture
{
    std::string name;
    std::string pictureName;
    uint8_t texCoordIndex;
    AddressMode addressU;
    AddressMode addressV;
    std::string type;

    Texture();

    void write(SampleChunk& out) const;
};
