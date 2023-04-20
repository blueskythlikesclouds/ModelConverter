#pragma once

struct SampleChunkWriter;

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

    void write(SampleChunkWriter& writer) const;
    bool save(const char* path) const;
};
