#pragma once

struct SampleChunk;
struct Mesh;

struct MeshGroup
{
    std::vector<Mesh> opaqueMeshes;
    std::vector<Mesh> transparentMeshes;
    std::vector<Mesh> punchThroughMeshes;
    std::unordered_map<std::string, std::vector<Mesh>> specialMeshGroups;

    std::string name;

    MeshGroup();
    ~MeshGroup();

    void write(SampleChunk& out) const;
};
