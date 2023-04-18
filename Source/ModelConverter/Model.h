#pragma once

struct SampleChunk;
struct Node;
struct MeshGroup;

struct Model
{
    std::vector<MeshGroup> meshGroups;
    std::vector<Node> nodes;
    Float3 min;
    Float3 max;

    Model();
    ~Model();

    void write(SampleChunk& out) const;
    void save(const char* path) const;
};
