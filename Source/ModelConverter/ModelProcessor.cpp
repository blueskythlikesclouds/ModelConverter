#include "ModelProcessor.h"

#include "Config.h"
#include "Mesh.h"
#include "MeshGroup.h"
#include "Model.h"
#include "TextureUnit.h"
#include "VertexElement.h"

template<typename T>
static void traverseModel(Model& model, const T& function)
{
    for (auto& meshGroup : model.meshGroups)
    {
        for(size_t i = 0; i < meshGroup.opaqueMeshes.size(); i++)
            function(meshGroup.opaqueMeshes[i], meshGroup.opaqueMeshes);

        for (size_t i = 0; i < meshGroup.transparentMeshes.size(); i++)
            function(meshGroup.transparentMeshes[i], meshGroup.transparentMeshes);

        for (size_t i = 0; i < meshGroup.punchThroughMeshes.size(); i++)
            function(meshGroup.punchThroughMeshes[i], meshGroup.punchThroughMeshes);

        for (auto& group : meshGroup.specialMeshGroups | std::views::values)
        {
            for (size_t i = 0; i < group.size(); i++)
                function(group[i], group);
        }
    }
}

static void unifyModel(Model& model)
{
    traverseModel(model, [](Mesh& mesh, auto&)
    {
        std::vector<meshopt_Stream> streams;

        for (auto& vertexStream : mesh.vertexStreams)
        {
            for (auto& stream : vertexStream)
            {
                if (!stream.empty())
                    streams.emplace_back(stream.data(), sizeof(Vector4), sizeof(Vector4));
            }
        }

        std::vector<unsigned int> remap(mesh.faceIndices.size());
        
        const size_t vertexCount = meshopt_generateVertexRemapMulti(remap.data(),
            mesh.faceIndices.data(), mesh.faceIndices.size(), mesh.vertexStreams[0][0].size(), streams.data(), streams.size());
        
        for (auto& vertexStream : mesh.vertexStreams)
        {
            for (auto& stream : vertexStream)
            {
                if (stream.empty())
                    continue;
        
                meshopt_remapVertexBuffer(stream.data(), stream.data(), stream.size(), sizeof(Vector4), remap.data());
                stream.resize(vertexCount);
            }
        }
        
        meshopt_remapIndexBuffer(mesh.faceIndices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), remap.data());
    });
}

static void splitModel(Model& model, Config config)
{
    traverseModel(model, [config](Mesh& mesh, auto& group)
    {
        const size_t maxVertices = (config & CONFIG_FLAG_TRIANGLE_LIST) ? 65536 : 65535;
        constexpr size_t MAX_NODES = 25;

        if (mesh.vertexStreams[0][0].size() <= maxVertices && ((config & CONFIG_FLAG_NODE_LIMIT) == 0 || mesh.nodeIndices.size() <= MAX_NODES))
            return;

        std::unordered_set<uint32_t> vertices;
        std::unordered_set<uint32_t> nodes;

        for (size_t i = 0; i < mesh.faceIndices.size(); i += 3)
        {
            for (size_t j = 0; j < 3; j++)
            {
                vertices.insert(mesh.faceIndices[i + j]);

                if (config & CONFIG_FLAG_NODE_LIMIT)
                {
                    for (auto& stream : mesh.vertexStreams[static_cast<size_t>(VertexType::BlendIndices)])
                    {
                        for (const auto index : stream[mesh.faceIndices[i + j]].u)
                        {
                            if (index != static_cast<uint32_t>(-1))
                                nodes.insert(index);
                        }
                    }
                }
            }

            if (vertices.size() > maxVertices || ((config & CONFIG_FLAG_NODE_LIMIT) != 0 && nodes.size() > MAX_NODES))
            {
                Mesh copy(mesh);

                copy.faceIndices.erase(copy.faceIndices.begin(), copy.faceIndices.begin() + static_cast<ptrdiff_t>(i));
                mesh.faceIndices.erase(mesh.faceIndices.begin() + static_cast<ptrdiff_t>(i), mesh.faceIndices.end());

                group.push_back(std::move(copy));

                break;
            }
        }
    });
}

static void optimizeModel(Model& model, Config config)
{
    traverseModel(model, [config](Mesh& mesh, auto&)
    {
        size_t vertexCount = mesh.vertexStreams[0][0].size();

        if (config & CONFIG_FLAG_TRIANGLE_LIST)
            meshopt_optimizeVertexCache(mesh.faceIndices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount);
        else
            meshopt_optimizeVertexCacheStrip(mesh.faceIndices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount);

        std::vector<unsigned int> remap(vertexCount);
        vertexCount = meshopt_optimizeVertexFetchRemap(remap.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount);

        for (auto& vertexStream : mesh.vertexStreams)
        {
            for (auto& stream : vertexStream)
            {
                if (stream.empty())
                    continue;

                meshopt_remapVertexBuffer(stream.data(), stream.data(), stream.size(), sizeof(Vector4), remap.data());
                stream.resize(vertexCount);
            }
        }

        meshopt_remapIndexBuffer(mesh.faceIndices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), remap.data());

        if ((config & CONFIG_FLAG_TRIANGLE_LIST) == 0)
        {
            std::vector<uint32_t> indices;
            indices.resize(meshopt_stripifyBound(mesh.faceIndices.size()));
            indices.resize(meshopt_stripify(indices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount, static_cast<uint32_t>(-1)));
            std::swap(mesh.faceIndices, indices);
        }

        if (config & (CONFIG_FLAG_XBOX_VERTEX_FORMAT | CONFIG_FLAG_D3D11_VERTEX_FORMAT))
        {
            for (auto& vertexElement : mesh.vertexElements)
            {
                switch (vertexElement.type)
                {
                case VertexType::Normal:
                case VertexType::Tangent:
                case VertexType::Binormal:
                    vertexElement.format = (config & CONFIG_FLAG_D3D11_VERTEX_FORMAT) ? VertexFormat::DEC3N : VertexFormat::HEND3N;
                    break;

                case VertexType::TexCoord:
                    vertexElement.format = VertexFormat::FLOAT16_2;
                    break;

                case VertexType::Color:
                    vertexElement.format = VertexFormat::UBYTE4N;
                    break;
                }
            }

            mesh.vertexElements[0].offset = 0;

            for (size_t i = 1; i < mesh.vertexElements.size(); i++)
                mesh.vertexElements[i].offset = static_cast<uint16_t>(mesh.vertexElements[i - 1].getNextOffset());
        }

        std::unordered_map<uint32_t, uint16_t> nodeRemap;

        for (auto& vertexStream : mesh.vertexStreams[static_cast<size_t>(VertexType::BlendIndices)])
        {
            for (auto& indices : vertexStream)
            {
                for (auto& index : indices.u)
                {
                    if (index == static_cast<uint32_t>(-1))
                        continue;

                    const auto pair = nodeRemap.find(index);
                    if (pair != nodeRemap.end())
                    {
                        index = pair->second;
                    }
                    else
                    {
                        const uint16_t newIndex = static_cast<uint16_t>(nodeRemap.size());
                        nodeRemap.emplace(index, newIndex);
                        index = newIndex;
                    }
                }
            }
        }

        std::vector<uint16_t> nodeIndices(nodeRemap.size());

        for (const auto& [meshNodeIndex, newIndex] : nodeRemap)
            nodeIndices[newIndex] = mesh.nodeIndices[meshNodeIndex];

        std::swap(mesh.nodeIndices, nodeIndices);
    });
}

struct MikkTSpaceContext
{
    std::vector<size_t> offsets;
    std::vector<Mesh*> meshes;
    size_t indexCount;

    MikkTSpaceContext() : indexCount()
    {
    }

    Vector4& get(VertexType type, size_t index) const
    {
        const auto it = std::ranges::upper_bound(offsets, index) - 1;
        return meshes[it - offsets.begin()]->vertexStreams[static_cast<size_t>(type)][0][index - *it];
    }
};

static int getNumFaces(const SMikkTSpaceContext* pContext)
{
    return static_cast<int>(static_cast<MikkTSpaceContext*>(pContext->m_pUserData)->indexCount / 3);
}

static int getNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace)
{
    return 3;
}

static void getPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
{
    const Vector4& position =
        static_cast<MikkTSpaceContext*>(pContext->m_pUserData)->get(VertexType::Position, iFace * 3 + iVert);

    fvPosOut[0] = position.fx;
    fvPosOut[1] = position.fy;
    fvPosOut[2] = position.fz;
}

static void getNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
{
    const Vector4& normal =
        static_cast<MikkTSpaceContext*>(pContext->m_pUserData)->get(VertexType::Normal, iFace * 3 + iVert);

    fvNormOut[0] = normal.fx;
    fvNormOut[1] = normal.fy;
    fvNormOut[2] = normal.fz;
}

static void getTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
{
    const Vector4& texCoord =
        static_cast<MikkTSpaceContext*>(pContext->m_pUserData)->get(VertexType::TexCoord, iFace * 3 + iVert);

    fvTexcOut[0] = texCoord.fx;
    fvTexcOut[1] = texCoord.fy;
}

static void setTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
{
    Vector4& normal = 
        static_cast<MikkTSpaceContext*>(pContext->m_pUserData)->get(VertexType::Normal, iFace * 3 + iVert);

    Vector4& tangent =
        static_cast<MikkTSpaceContext*>(pContext->m_pUserData)->get(VertexType::Tangent, iFace * 3 + iVert);

    Vector4& binormal =
        static_cast<MikkTSpaceContext*>(pContext->m_pUserData)->get(VertexType::Binormal, iFace * 3 + iVert);

    tangent.fx = fvTangent[0];
    tangent.fy = fvTangent[1];
    tangent.fz = fvTangent[2];
    tangent.fw = 0.0f;

    binormal.fx = (normal.fy * tangent.fz - normal.fz * tangent.fy) * fSign;
    binormal.fy = (normal.fz * tangent.fx - normal.fx * tangent.fz) * fSign;
    binormal.fz = (normal.fx * tangent.fy - normal.fy * tangent.fx) * fSign;
    binormal.fw = 0.0f;

    const float magnitude = (binormal.fx * binormal.fx) + (binormal.fy * binormal.fy) + (binormal.fz * binormal.fz);
    if (magnitude > 0.0f)
    {
        const float invSqrt = 1.0f / sqrtf(magnitude);
        binormal.fx *= invSqrt;
        binormal.fy *= invSqrt;
        binormal.fz *= invSqrt;
    }
}

static SMikkTSpaceInterface interface = { getNumFaces, getNumVerticesOfFace, getPosition, getNormal, getTexCoord, setTSpaceBasic, nullptr };

static void generateTangents(Model& model)
{
    MikkTSpaceContext context;

    traverseModel(model, [&](Mesh& mesh, auto&)
    {
        auto& normals = mesh.vertexStreams[static_cast<size_t>(VertexType::Normal)];
        if (!normals.empty() && !normals[0].empty())
        {
            context.offsets.push_back(context.indexCount);
            context.meshes.push_back(&mesh);
            context.indexCount += normals[0].size();
        }
    });

    SMikkTSpaceContext ctx = { &interface, &context };
    genTangSpaceDefault(&ctx);
}

void ModelProcessor::process(Model& model, Config config)
{
    generateTangents(model);
    unifyModel(model);
    splitModel(model, config);
    optimizeModel(model, config);
}
