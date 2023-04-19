#include "ModelConverter.h"

#include "Mesh.h"
#include "MeshGroup.h"
#include "ModelOptimizer.h"
#include "Node.h"
#include "TangentGenerator.h"
#include "TextureUnit.h"
#include "VertexElement.h"

ModelConverter::ModelConverter(const char* path)
{
    importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES, 24);
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 32767);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    aiScene = importer.ReadFile(path,
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_SplitLargeMeshes |
        aiProcess_LimitBoneWeights |
        aiProcess_SortByPType |
        aiProcess_SplitByBoneCount | 
        aiProcess_FlipUVs);
}

Mesh ModelConverter::convertMesh(const aiMesh* aiMesh, const aiMatrix4x4& matrix)
{
    aiQuaternion ori;
    aiVector3D pos;
    matrix.DecomposeNoScaling(ori, pos);

    Mesh mesh;
    mesh.materialName = aiScene->mMaterials[aiMesh->mMaterialIndex]->GetName().C_Str();
    mesh.faceIndices.reserve(aiMesh->mNumFaces * 3);

    for (size_t i = 0; i < aiMesh->mNumFaces; i++)
    {
        const aiFace& aiFace = aiMesh->mFaces[i];

        if (aiFace.mNumIndices == 3)
        {
            mesh.faceIndices.push_back(static_cast<uint16_t>(aiFace.mIndices[2]));
            mesh.faceIndices.push_back(static_cast<uint16_t>(aiFace.mIndices[1]));
            mesh.faceIndices.push_back(static_cast<uint16_t>(aiFace.mIndices[0]));
        }
    }

    auto& positions = mesh.vertexStreams[static_cast<size_t>(VertexType::Position)].emplace_back();
    positions.reserve(mesh.faceIndices.size());

    for (const auto index : mesh.faceIndices)
    {
        const aiVector3D position = matrix * aiMesh->mVertices[index];
        positions.emplace_back(position[0], position[1], position[2]);

        for (uint32_t j = 0; j < 3; j++)
        {
            model.min[j] = std::min(model.min[j], position[j]);
            model.max[j] = std::max(model.max[j], position[j]);
        }
    }

    mesh.vertexElements.emplace_back(0, VertexFormat::FLOAT3, VertexType::Position, 0);

    if (aiMesh->HasNormals())
    {
        auto& normals = mesh.vertexStreams[static_cast<size_t>(VertexType::Normal)].emplace_back();
        normals.reserve(mesh.faceIndices.size());

        for (const auto index : mesh.faceIndices)
        {
            const aiVector3D normal = ori.Rotate(aiMesh->mNormals[index]).NormalizeSafe();
            normals.emplace_back(normal[0], normal[1], normal[2]);
        }

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), VertexFormat::FLOAT3, VertexType::Normal, 0);

        auto& tangents = mesh.vertexStreams[static_cast<size_t>(VertexType::Tangent)].emplace_back();
        tangents.resize(mesh.faceIndices.size());

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), VertexFormat::FLOAT3, VertexType::Tangent, 0);

        auto& binormals = mesh.vertexStreams[static_cast<size_t>(VertexType::Binormal)].emplace_back();
        binormals.resize(mesh.faceIndices.size());

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), VertexFormat::FLOAT3, VertexType::Binormal, 0);
    }

    for (uint32_t i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
    {
        auto& texCoords = mesh.vertexStreams[static_cast<size_t>(VertexType::TexCoord)].emplace_back();

        if (!aiMesh->HasTextureCoords(i))
            continue;

        texCoords.reserve(mesh.faceIndices.size());

        for (const auto index : mesh.faceIndices)
        {
            const aiVector3D& texCoord = aiMesh->mTextureCoords[i][index];
            texCoords.emplace_back(texCoord[0], texCoord[1]);
        }

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), VertexFormat::FLOAT2, VertexType::TexCoord, i);
    }

    for (uint32_t i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; i++)
    {
        auto& colors = mesh.vertexStreams[static_cast<size_t>(VertexType::Color)].emplace_back();

        const bool hasVertexColors = aiMesh->HasVertexColors(i);
        if (!hasVertexColors && i != 0)
            continue;

        if (hasVertexColors)
        {
            colors.reserve(mesh.faceIndices.size());

            for (const auto index : mesh.faceIndices)
            {
                const aiColor4D& color = aiMesh->mColors[i][index];
                colors.emplace_back(color[0], color[1], color[2], color[3]);
            }
        }
        else
        {
            colors.resize(mesh.faceIndices.size(), Vector4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), VertexFormat::FLOAT4, VertexType::Color, i);
    }

    if (aiMesh->HasBones())
    {
        std::vector<Vector4> blendIndices, blendWeights;

        blendIndices.resize(aiMesh->mNumVertices, Vector4(-1, -1, -1, -1));
        blendWeights.resize(aiMesh->mNumVertices, Vector4(0.0f, 0.0f, 0.0f, 0.0f));

        for (size_t i = 0; i < aiMesh->mNumBones; i++)
        {
            const aiBone* aiBone = aiMesh->mBones[i];
            const auto modelNodeIndex = nodes.find(aiBone->mName.C_Str());

            if (modelNodeIndex == nodes.end())
                continue;

            const size_t meshNodeIndex = mesh.nodeIndices.size();
            mesh.nodeIndices.push_back(static_cast<uint16_t>(modelNodeIndex->second));

            for (size_t j = 0; j < aiBone->mNumWeights; j++)
            {
                const aiVertexWeight& aiWeight = aiBone->mWeights[j];

                int32_t index = -1;
                auto& blendIndex = blendIndices[aiWeight.mVertexId];

                for (int32_t k = 0; k < 4; k++)
                {
                    if (blendIndex.i[k] == -1)
                    {
                        index = k;
                        break;
                    }
                }

                if (index >= 0)
                {
                    blendIndex.u[index] = static_cast<uint32_t>(meshNodeIndex);
                    auto& blendWeight = blendWeights[aiWeight.mVertexId];
                    blendWeight.f[index] = aiWeight.mWeight;
                }
            }
        }

        auto& isolatedBlendIndices = mesh.vertexStreams[static_cast<size_t>(VertexType::BlendIndices)].emplace_back();
        isolatedBlendIndices.reserve(mesh.faceIndices.size());

        auto& isolatedBlendWeights = mesh.vertexStreams[static_cast<size_t>(VertexType::BlendWeight)].emplace_back();
        isolatedBlendWeights.reserve(mesh.faceIndices.size());

        for (const auto index : mesh.faceIndices)
        {
            isolatedBlendIndices.push_back(blendIndices[index]);
            isolatedBlendWeights.push_back(blendWeights[index]);
        }

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), VertexFormat::UBYTE4, VertexType::BlendIndices, 0);
        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), VertexFormat::UBYTE4N, VertexType::BlendWeight, 0);
    }

    for (size_t i = 0; i < mesh.faceIndices.size(); i++)
        mesh.faceIndices[i] = static_cast<uint16_t>(i);

    return mesh;
}

void ModelConverter::convertNodesRecursively(const aiNode* aiNode, size_t parentIndex, const aiMatrix4x4& parentMatrix)
{
    const aiMatrix4x4 matrix = parentMatrix * aiNode->mTransformation;

    if (aiNode != aiScene->mRootNode && aiNode->mNumMeshes == 0)
    {
        const size_t nodeIndex = model.nodes.size();

        auto& node = model.nodes.emplace_back();
        node.parentIndex = static_cast<uint32_t>(parentIndex);
        node.name = aiNode->mName.C_Str();
        memcpy(&node.matrix, &aiMatrix4x4(matrix).Inverse(), sizeof(matrix));

        nodes.emplace(aiNode->mName.C_Str(), nodeIndex);
        parentIndex = nodeIndex;
    }

    for (size_t i = 0; i < aiNode->mNumChildren; i++)
        convertNodesRecursively(aiNode->mChildren[i], parentIndex, matrix);
}

void ModelConverter::convertMeshesRecursively(const aiNode* aiNode, const aiMatrix4x4& parentMatrix)
{
    const aiMatrix4x4 matrix = parentMatrix * aiNode->mTransformation;

    if (aiNode->mNumMeshes != 0)
    {
        MeshGroup& group = model.meshGroups.emplace_back();

        for (size_t i = 0; i < aiNode->mNumMeshes; i++)
        {
            const aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
            if ((aiMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
                continue;

            Mesh mesh = convertMesh(aiMesh, matrix);

            if (!mesh.faceIndices.empty() && !mesh.vertexStreams[0].empty() && !mesh.vertexStreams[0][0].empty())
                group.opaqueMeshes.emplace_back(std::move(mesh));
        }

        group.name = aiNode->mName.C_Str();
    }

    for (size_t i = 0; i < aiNode->mNumChildren; i++)
        convertMeshesRecursively(aiNode->mChildren[i], matrix);
}

Model&& ModelConverter::convert()
{
    convertNodesRecursively(aiScene->mRootNode, ~0, aiMatrix4x4());
    convertMeshesRecursively(aiScene->mRootNode, aiMatrix4x4());
    TangentGenerator::generate(model);
    ModelOptimizer::optimize(model);
    return std::move(model);
}
