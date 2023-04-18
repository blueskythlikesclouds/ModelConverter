#pragma once

#include "Model.h"

struct Mesh;

struct ModelConverter
{
    Assimp::Importer importer;
    const aiScene* aiScene;

    Model model;
    std::unordered_map<std::string, size_t> nodes;

    ModelConverter(const char* path);

    Mesh convertMesh(const aiMesh* aiMesh, const aiMatrix4x4& matrix);

    void convertNodesRecursively(const aiNode* aiNode, size_t parentIndex, const aiMatrix4x4& parentMatrix);
    void convertMeshesRecursively(const aiNode* aiNode, const aiMatrix4x4& parentMatrix);

    Model&& convert();
};
