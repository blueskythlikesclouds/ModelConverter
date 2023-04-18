#pragma once

struct SampleChunk;
struct Mesh;

struct MeshGroup
{
    std::vector<Mesh> opaqueMeshes;
    std::vector<Mesh> transparentMeshes;
    std::vector<Mesh> punchThroughMeshes;

    std::vector<std::string> specialMeshGroupTypes;
    std::vector<std::vector<Mesh>> specialMeshGroups;

    std::string name;

    MeshGroup();
    ~MeshGroup();

    void write(SampleChunk& out) const;
};
