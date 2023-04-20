﻿#pragma once

#include "Model.h"

enum Config;

struct Material;
struct Mesh;

struct ModelConverter
{
    Assimp::Importer importer;
    const aiScene* aiScene;

    Model model;
    std::vector<Material> materials;
    std::unordered_map<std::string_view, size_t> nodeIndices;

    ModelConverter(const char* path, Config config);
    ~ModelConverter();

    Mesh convertMesh(const aiMesh* aiMesh, const aiMatrix4x4& matrix);

    void convertMaterial(const aiMaterial* aiMaterial);
    void convertMaterials();

    void convertNodesRecursively(const aiNode* aiNode, size_t parentIndex, const aiMatrix4x4& parentMatrix);
    void convertNodes();

    void convertMeshesRecursively(const aiNode* aiNode, const aiMatrix4x4& parentMatrix);
    void convertMeshes();

    void convert(Config config);
};
