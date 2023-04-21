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
        converter.convertMaterials();
        converter.convertNodes();
        converter.convertMeshes();

        ModelProcessor::process(holder.model, config);

        return true;
    }

    return false;
}

Mesh ModelConverter::convertMesh(const aiMesh* aiMesh, const aiMatrix4x4& matrix)
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

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), VertexFormat::UBYTE4N, VertexType::Color, i);
    }

    if (aiMesh->HasBones())
    {
        std::vector<Vector4> blendIndices, blendWeights;

        blendIndices.resize(aiMesh->mNumVertices, Vector4(-1, -1, -1, -1));
        blendWeights.resize(aiMesh->mNumVertices, Vector4(0.0f, 0.0f, 0.0f, 0.0f));

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

                auto& blendIndex = blendIndices[aiWeight.mVertexId];
                auto& blendWeight = blendWeights[aiWeight.mVertexId];

                for (int32_t i = 0; i < 4; i++)
                {
                    if (aiWeight.mWeight > blendWeight.f[i])
                    {
                        for (int32_t j = 3; j > i; j--)
                        {
                            blendIndex.u[j] = blendIndex.u[j - 1];
                            blendWeight.f[j] = blendWeight.f[j - 1];
                        }

                        blendIndex.u[i] = static_cast<uint32_t>(meshNodeIndex);
                        blendWeight.f[i] = aiWeight.mWeight;

                        break;
                    }
                }
            }
        }

        for (size_t i = 0; i < aiMesh->mNumVertices; i++)
        {
            auto& blendIndex = blendIndices[i];
            auto& blendWeight = blendWeights[i];

            float totalWeight = blendWeight.fx + blendWeight.fy + blendWeight.fz + blendWeight.fw;

            if (totalWeight > 0.0f)
            {
                totalWeight = 1.0f / totalWeight;

                blendWeight.fx *= totalWeight;
                blendWeight.fy *= totalWeight;
                blendWeight.fz *= totalWeight;
                blendWeight.fw *= totalWeight;
            }
            else
            {
                blendIndex.i[0] = 0;
                blendIndex.i[1] = -1;
                blendIndex.i[2] = -1;
                blendIndex.i[3] = -1;
            }

            blendIndex.reverse();
            blendWeight.reverse();
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

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), 
            holder.model.nodes.size() > 255 ? VertexFormat::USHORT4 : VertexFormat::UBYTE4, VertexType::BlendIndices, 0);

        mesh.vertexElements.emplace_back(mesh.vertexElements.back().getNextOffset(), 
            VertexFormat::UBYTE4N, VertexType::BlendWeight, 0);
    }

    for (size_t i = 0; i < mesh.faceIndices.size(); i++)
        mesh.faceIndices[i] = static_cast<uint32_t>(i);

    auto& material = holder.materials[aiMesh->mMaterialIndex];

    for (const auto& texture : material.textures)
        mesh.textureUnits.emplace_back(texture.type, static_cast<uint8_t>(mesh.textureUnits.size()));

    return mesh;
}

void ModelConverter::convertMaterial(const aiMaterial* aiMaterial)
{
    const aiString str = aiMaterial->GetName();
    const Tags tags(str.C_Str());

    auto& material = holder.materials.emplace_back();

    material.name = tags.name;
    material.shader = tags.getValue("SHDR", 0, "Common_d");
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
            material.scaParameters.emplace(name, tag.getIntValue(1, NULL));
    }
}

void ModelConverter::convertMaterials()
{
    for (size_t i = 0; i < aiScene->mNumMaterials; i++)
        convertMaterial(aiScene->mMaterials[i]);
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

void ModelConverter::convertMeshesRecursively(const aiNode* aiNode, const aiMatrix4x4& parentMatrix)
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

            Mesh mesh = convertMesh(aiMesh, matrix);

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
                holder.model.scaParameters.emplace(name, tag.getIntValue(1, NULL));
        }
    }

    for (size_t i = 0; i < aiNode->mNumChildren; i++)
        convertMeshesRecursively(aiNode->mChildren[i], matrix);
}

void ModelConverter::convertMeshes()
{
    convertMeshesRecursively(aiScene->mRootNode, aiMatrix4x4());
}