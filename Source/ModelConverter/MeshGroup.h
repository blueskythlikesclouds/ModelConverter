#pragma once

struct SampleChunkWriter;
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

    void write(SampleChunkWriter& writer, uint32_t dataVersion) const;
};
