#pragma once

enum Config;

struct ModelHolder;
struct Mesh;

struct ModelConverter
{
    Assimp::Importer importer;
    const aiScene* aiScene;

    ModelHolder& holder;

    std::unordered_map<std::string_view, size_t> meshGroupIndices;
    std::unordered_map<std::string_view, size_t> nodeIndices;

    ModelConverter(ModelHolder& holder);
    ~ModelConverter();

    static bool convert(const char* path, Config config, ModelHolder& holder);

    Mesh convertMesh(const aiMesh* aiMesh, const aiMatrix4x4& matrix);

    void convertMaterial(const aiMaterial* aiMaterial);
    void convertMaterials();

    void convertNodesRecursively(const aiNode* aiNode, size_t parentIndex, const aiMatrix4x4& parentMatrix);
    void convertNodes();

    void convertMeshesRecursively(const aiNode* aiNode, const aiMatrix4x4& parentMatrix);
    void convertMeshes();
};
