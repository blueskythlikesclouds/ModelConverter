#pragma once

enum Config;

struct SampleChunkWriter;

template<typename T>
struct Parameter;

struct Texture;

struct Material
{
    std::string name;
    std::string shader;
    std::string layer;
    uint8_t alphaThreshold;
    bool doubleSided;
    bool additive;
    std::vector<Texture> textures;
    std::vector<Parameter<Float4>> float4Parameters;
    std::vector<Parameter<Int4>> int4Parameters;
    std::vector<Parameter<BOOL>> boolParameters;
    std::unordered_map<std::string, int> scaParameters;

    Material();
    ~Material();

    void write(SampleChunkWriter& writer, uint32_t dataVersion) const;
    void save(const char* path, Config config) const;
};
