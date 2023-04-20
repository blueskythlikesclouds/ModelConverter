#pragma once

enum Config;

struct SampleChunkWriter;
struct Node;
struct MeshGroup;

struct Model
{
    std::vector<MeshGroup> meshGroups;
    std::vector<Node> nodes;
    Float3 min;
    Float3 max;
    std::unordered_map<std::string, int> scaParameters;

    Model();
    ~Model();

    void write(SampleChunkWriter& writer, uint32_t dataVersion) const;
    bool save(const char* path, Config config) const;
};
