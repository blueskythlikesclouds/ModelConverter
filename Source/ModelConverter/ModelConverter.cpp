#include "ModelConverter.h"

#include "Config.h"
#include "Mesh.h"
#include "MeshGroup.h"
#include "ModelHolder.h"
#include "ModelProcessor.h"
#include "Node.h"
#include "Parameter.h"
#include "Tag.h"
#include "Texture.h"
#include "TextureUnit.h"
#include "VertexElement.h"

ModelConverter::ModelConverter(ModelHolder& holder)
    : aiScene()
    , holder(holder)
{
}

ModelConverter::~ModelConverter() = default;

bool ModelConverter::convert(const char* path, Config config, ModelHolder& holder)
{
    ModelConverter converter(holder);

    converter.importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    converter.aiScene = converter.importer.ReadFile(path, aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_FlipUVs);

    if (converter.aiScene)
    {
        if ((config & CONFIG_FLAG_V2_SAMPLE_CHUNK) && (config & CONFIG_FLAG_D3D11_VERTEX_FORMAT))
        {
            holder.model.scaParameters.emplace("ShadowCa", true);
            holder.model.scaParameters.emplace("ShadowRe", true);
        }

        converter.convertMaterials(config);
        converter.convertNodes();
        converter.convertMeshes(config);

        ModelProcessor::process(holder.model, config);

        return true;
    }

    return false;
}

Mesh ModelConverter::convertMesh(const aiMesh* aiMesh, const aiMatrix4x4& matrix, Config config)
{
    aiQuaternion ori;
    aiVector3D pos;
    matrix.DecomposeNoScaling(ori, pos);

    Mesh mesh;
    mesh.materialName = Tags::getName(aiScene->mMaterials[aiMesh->mMaterialIndex]->GetName().C_Str());
    mesh.faceIndices.reserve(aiMesh->mNumFaces * 3);

    for (size_t i = 0; i < aiMesh->mNumFaces; i++)
    {
        const aiFace& aiFace = aiMesh->mFaces[i];

        if (aiFace.mNumIndices == 3)
        {
            mesh.faceIndices.push_back(aiFace.mIndices[2]);
            mesh.faceIndices.push_back(aiFace.mIndices[1]);
            mesh.faceIndices.push_back(aiFace.mIndices[0]);
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
            holder.model.min[j] = std::min(holder.model.min[j], position[j]);
            holder.model.max[j] = std::max(holder.model.max[j], position[j]);
        }
    }

    if (aiMesh->HasNormals())
    {
        auto& normals = mesh.vertexStreams[static_cast<size_t>(VertexType::Normal)].emplace_back();
        normals.reserve(mesh.faceIndices.size());

        for (const auto index : mesh.faceIndices)
        {
            const aiVector3D normal = ori.Rotate(aiMesh->mNormals[index]).NormalizeSafe();
            normals.emplace_back(normal[0], normal[1], normal[2]);
        }

        auto& tangents = mesh.vertexStreams[static_cast<size_t>(VertexType::Tangent)].emplace_back();
        tangents.resize(mesh.faceIndices.size());

        auto& binormals = mesh.vertexStreams[static_cast<size_t>(VertexType::Binormal)].emplace_back();
        binormals.resize(mesh.faceIndices.size());
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
    }

    if (aiMesh->HasBones())
    {
        const uint32_t weightsPerVertex = config & CONFIG_FLAG_8_NODES_PER_VERTEX ? 8u : 4u;

        std::vector<std::vector<Vector4>> blendIndices(weightsPerVertex / 4);
        std::vector<std::vector<Vector4>> blendWeights(weightsPerVertex / 4);

        for (auto& blendIndex : blendIndices)
            blendIndex.resize(aiMesh->mNumVertices, Vector4(-1, -1, -1, -1));

        for (auto& blendWeight : blendWeights)
            blendWeight.resize(aiMesh->mNumVertices, Vector4(0.0f, 0.0f, 0.0f, 0.0f));

        #define BLEND_INDEX(WEIGHT_ID, VERTEX_ID) blendIndices[(WEIGHT_ID) / 4][VERTEX_ID].i[(WEIGHT_ID) % 4]
        #define BLEND_WEIGHT(WEIGHT_ID, VERTEX_ID) blendWeights[(WEIGHT_ID) / 4][VERTEX_ID].f[(WEIGHT_ID) % 4]

        for (size_t boneIndex = 0; boneIndex < aiMesh->mNumBones; boneIndex++)
        {
            const aiBone* aiBone = aiMesh->mBones[boneIndex];
            const auto modelNodeIndex = nodeIndices.find(aiBone->mName.C_Str());

            if (modelNodeIndex == nodeIndices.end())
                continue;

            const size_t meshNodeIndex = mesh.nodeIndices.size();
            mesh.nodeIndices.push_back(static_cast<uint16_t>(modelNodeIndex->second));

            for (size_t weightIndex = 0; weightIndex < aiBone->mNumWeights; weightIndex++)
            {
                const aiVertexWeight& aiWeight = aiBone->mWeights[weightIndex];

                for (int32_t i = 0; i < static_cast<int32_t>(weightsPerVertex); i++)
                {
                    if (aiWeight.mWeight > BLEND_WEIGHT(i, aiWeight.mVertexId))
                    {
                        for (int32_t j = static_cast<int32_t>(weightsPerVertex) - 1; j > i; j--)
                        {
                            BLEND_INDEX(j, aiWeight.mVertexId) = BLEND_INDEX(j - 1, aiWeight.mVertexId);
                            BLEND_WEIGHT(j, aiWeight.mVertexId) = BLEND_WEIGHT(j - 1, aiWeight.mVertexId);
                        }

                        BLEND_INDEX(i, aiWeight.mVertexId) = static_cast<uint32_t>(meshNodeIndex);
                        BLEND_WEIGHT(i, aiWeight.mVertexId) = aiWeight.mWeight;

                        break;
                    }
                }
            }
        }

        uint32_t maxWeightsPerVertex = 0;

        for (size_t i = 0; i < aiMesh->mNumVertices; i++)
        {
            uint32_t numWeights = 0;
            float totalWeight = 0.0f;

            for (size_t j = 0; j < weightsPerVertex; j++)
            {
                if (BLEND_INDEX(j, i) >= 0)
                    ++numWeights;

                totalWeight += BLEND_WEIGHT(j, i);
            }

            maxWeightsPerVertex = std::max(maxWeightsPerVertex, numWeights);

            if (totalWeight > 0.0f)
            {
                totalWeight = 1.0f / totalWeight;

                for (size_t j = 0; j < numWeights; j++)
                    BLEND_WEIGHT(j, i) *= totalWeight;
            }
            else
            {
                for (size_t j = 0; j < weightsPerVertex; j++)
                    BLEND_INDEX(j, i) = (j == 0) ? 0 : -1;
            }

            for (auto& blendIndex : blendIndices)
                blendIndex[i].reverse();

            for (auto& blendWeight : blendWeights)
                blendWeight[i].reverse();
        }

        #undef BLEND_WEIGHT
        #undef BLEND_INDEX

        for (uint32_t i = 0; i < (maxWeightsPerVertex + 3) / 4; i++)
        {
            auto& isolatedBlendIndices = mesh.vertexStreams[static_cast<size_t>(VertexType::BlendIndices)].emplace_back();
            isolatedBlendIndices.reserve(mesh.faceIndices.size());

            auto& isolatedBlendWeights = mesh.vertexStreams[static_cast<size_t>(VertexType::BlendWeight)].emplace_back();
            isolatedBlendWeights.reserve(mesh.faceIndices.size());

            for (const auto index : mesh.faceIndices)
            {
                isolatedBlendIndices.push_back(blendIndices[i][index]);
                isolatedBlendWeights.push_back(blendWeights[i][index]);
            }
        }
    }

    for (size_t i = 0; i < mesh.faceIndices.size(); i++)
        mesh.faceIndices[i] = static_cast<uint32_t>(i);

    auto& material = holder.materials[aiMesh->mMaterialIndex];

    for (const auto& texture : material.textures)
        mesh.textureUnits.emplace_back(texture.type, static_cast<uint8_t>(mesh.textureUnits.size()));

    return mesh;
}

void ModelConverter::convertMaterial(const aiMaterial* aiMaterial, Config config)
{
    const aiString str = aiMaterial->GetName();
    const Tags tags(str.C_Str());

    auto& material = holder.materials.emplace_back();

    material.name = tags.name;
    material.shader = tags.getValue("SHDR", 0, (config & CONFIG_FLAG_V1_MATERIAL) ? "Common_d[b]" : "Common_d");
    material.layer = tags.getValue("LYR", 0, "solid");
    material.alphaThreshold = 128;
    material.doubleSided = !tags.getBoolValue("CULL", 0, true);
    material.additive = tags.getBoolValue("ADD", 0, false);

    for (size_t i = 0; i <= AI_TEXTURE_TYPE_MAX; i++)
    {
        const uint32_t count = aiMaterial->GetTextureCount(static_cast<aiTextureType>(i));
        if (count == 0)
            continue;

        std::string type;

        switch (i)
        {
        case aiTextureType_DIFFUSE: type = "diffuse"; break;
        case aiTextureType_SPECULAR: type = "specular"; break;
        case aiTextureType_AMBIENT: type = "ambient"; break;
        case aiTextureType_EMISSIVE: type = "emission"; break;
        case aiTextureType_HEIGHT: type = "height"; break;
        case aiTextureType_NORMALS: type = "normal"; break;
        case aiTextureType_SHININESS: type = "gloss"; break;
        case aiTextureType_OPACITY: type = "opacity"; break;
        case aiTextureType_DISPLACEMENT: type = "displacement"; break;
        case aiTextureType_REFLECTION: type = "reflection"; break;
        default: type = "unknown"; break;
        }

        for (uint32_t j = 0; j < count; j++)
        {
            aiString path;
            uint32_t uv = 0;
            aiTextureMapMode mode = aiTextureMapMode_Wrap;

            aiMaterial->GetTexture(static_cast<aiTextureType>(i), j, &path, nullptr, &uv, nullptr, nullptr, &mode);

            auto& texture = material.textures.emplace_back();

            texture.pictureName = path.C_Str();

            size_t index = texture.pictureName.find_last_of("\\/");
            if (index != std::string::npos)
                texture.pictureName.erase(0, index + 1);

            index = texture.pictureName.find_last_of('.');
            if (index != std::string::npos)
                texture.pictureName.erase(index);

            texture.texCoordIndex = static_cast<uint8_t>(uv);

            AddressMode addressMode = AddressMode::Repeat;

            switch (mode)
            {
            case aiTextureMapMode_Wrap: addressMode = AddressMode::Repeat; break;
            case aiTextureMapMode_Clamp: addressMode = AddressMode::Clamp; break;
            case aiTextureMapMode_Decal: addressMode = AddressMode::Border; break;
            case aiTextureMapMode_Mirror: addressMode = AddressMode::Mirror; break;
            }

            texture.addressU = addressMode;
            texture.addressV = addressMode;

            texture.type = type;
        }
    }

    auto range = tags.indices.equal_range("TXTR");

    for (auto it = range.first; it != range.second; ++it)
    {
        const Tag& tag = tags[it->second];

        auto& texture = material.textures.emplace_back();
        texture.type = tag.getValue(0, std::string_view());
        texture.pictureName = tag.getValue(1, std::string_view());

        const size_t index = texture.pictureName.find_last_of('.');
        if (index != std::string::npos)
            texture.pictureName.erase(index);
    }

    for (uint32_t i = 0; i < material.textures.size(); i++)
    {
        char num[16];
        sprintf(num, "-%04d", i);

        material.textures[i].name = material.name + num;
    }

    material.float4Parameters.push_back(Parameter<Float4>("diffuse", { { 1.0f, 1.0f, 1.0f, 0.0f } }));
    material.float4Parameters.push_back(Parameter<Float4>("ambient", { { 1.0f, 1.0f, 1.0f, 0.0f } }));
    material.float4Parameters.push_back(Parameter<Float4>("specular", { { 0.9f, 0.9f, 0.9f, 0.0f } }));
    material.float4Parameters.push_back(Parameter<Float4>("emissive", { { 0.0f, 0.0f, 0.0f, 0.0f } }));
    material.float4Parameters.push_back(Parameter<Float4>("power_gloss_level", { { 50.0f, 0.1f, 0.01f, 0.0f } }));
    material.float4Parameters.push_back(Parameter<Float4>("opacity_reflection_refraction_spectype", { { 1.0f, 0.0f, 1.0f, 0.0f } }));

    if (config & CONFIG_FLAG_D3D11_VERTEX_FORMAT)
        material.float4Parameters.push_back(Parameter<Float4>("PBRFactor", { { 0.0f, 0.0f, 0.0f, 0.0f } }));

    std::unordered_map<std::string_view, size_t> indices;

    for (auto& parameter : material.float4Parameters)
        indices.emplace(parameter.name, indices.size());

    range = tags.indices.equal_range("PMTR");

    for (auto it = range.first; it != range.second; ++it)
    {
        const Tag& tag = tags[it->second];

        const auto name = tag.getValue(0, std::string_view());
        if (name.empty())
            continue;

        size_t index;

        if (auto pair = indices.find(name); pair != indices.end())
        {
            index = pair->second;
        }
        else
        {
            index = material.float4Parameters.size();
            material.float4Parameters.push_back(Parameter<Float4>(std::string(name), { { 1.0f, 1.0f, 1.0f, 0.0f } }));
        }

        for (size_t i = 1; i < tag.values.size(); i++)
        {
            auto& value = material.float4Parameters[index].values[0][i - 1];
            value = tag.getFloatValue(i, value);
        }
    }

    range = tags.indices.equal_range("PRP");

    for (auto it = range.first; it != range.second; ++it)
    {
        const Tag& tag = tags[it->second];
        const auto name = tag.getValue(0, std::string_view());

        if (!name.empty())
            material.scaParameters[std::string(name)] = tag.getIntValue(1, NULL);
    }
}

void ModelConverter::convertMaterials(Config config)
{
    for (size_t i = 0; i < aiScene->mNumMaterials; i++)
        convertMaterial(aiScene->mMaterials[i], config);
}

void ModelConverter::convertNodesRecursively(const aiNode* aiNode, size_t parentIndex, const aiMatrix4x4& parentMatrix)
{
    const aiMatrix4x4 matrix = parentMatrix * aiNode->mTransformation;

    if (aiNode != aiScene->mRootNode && aiNode->mNumMeshes == 0)
    {
        const size_t nodeIndex = holder.model.nodes.size();

        auto& node = holder.model.nodes.emplace_back();
        node.parentIndex = static_cast<uint32_t>(parentIndex);
        node.name = aiNode->mName.C_Str();
        memcpy(&node.matrix, &aiMatrix4x4(matrix).Inverse(), sizeof(matrix));

        parentIndex = nodeIndex;
    }

    for (size_t i = 0; i < aiNode->mNumChildren; i++)
        convertNodesRecursively(aiNode->mChildren[i], parentIndex, matrix);
}

void ModelConverter::convertNodes()
{
    convertNodesRecursively(aiScene->mRootNode, ~0, aiMatrix4x4());

    for (const auto& node : holder.model.nodes)
        nodeIndices.emplace(node.name, nodeIndices.size());
}

void ModelConverter::convertMeshesRecursively(const aiNode* aiNode, const aiMatrix4x4& parentMatrix, Config config)
{
    const aiMatrix4x4 matrix = parentMatrix * aiNode->mTransformation;

    if (aiNode->mNumMeshes != 0)
    {
        const Tags tags(aiNode->mName.C_Str());
        const auto name = tags.getValue("NAME", 0, std::string_view());

        size_t index;

        if (const auto pair = meshGroupIndices.find(name); pair != meshGroupIndices.end())
        {
            index = pair->second;
        }
        else
        {
            index = holder.model.meshGroups.size();

            auto& group = holder.model.meshGroups.emplace_back();
            group.name = name;

            meshGroupIndices.emplace(name, index);
        }

        MeshGroup& group = holder.model.meshGroups[index];

        for (size_t i = 0; i < aiNode->mNumMeshes; i++)
        {
            const aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
            if ((aiMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
                continue;

            Mesh mesh = convertMesh(aiMesh, matrix, config);

            if (!mesh.faceIndices.empty() && !mesh.vertexStreams[0].empty() && !mesh.vertexStreams[0][0].empty())
            {
                auto& material = holder.materials[aiMesh->mMaterialIndex];

                if (material.layer == "solid")
                    group.opaqueMeshes.push_back(std::move(mesh));

                else if (material.layer == "trans")
                    group.transparentMeshes.push_back(std::move(mesh));

                else if (material.layer == "punch")
                    group.punchThroughMeshes.push_back(std::move(mesh));

                else
                    group.specialMeshGroups[material.layer].push_back(std::move(mesh));
            }
        }

        auto range = tags.indices.equal_range("PRP");
        for (auto it = range.first; it != range.second; ++it)
        {
            const Tag& tag = tags[it->second];
            const auto name = tag.getValue(0, std::string_view());

            if (!name.empty())
                holder.model.scaParameters[std::string(name)] = tag.getIntValue(1, NULL);
        }
    }

    for (size_t i = 0; i < aiNode->mNumChildren; i++)
        convertMeshesRecursively(aiNode->mChildren[i], matrix, config);
}

void ModelConverter::convertMeshes(Config config)
{
    convertMeshesRecursively(aiScene->mRootNode, aiMatrix4x4(), config);
}