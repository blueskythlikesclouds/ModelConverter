#include "ModelOptimizer.h"

#include "Config.h"
#include "Mesh.h"
#include "MeshGroup.h"
#include "Model.h"
#include "VertexElement.h"

static void optimizeMesh(Mesh& mesh, Config config)
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

    size_t vertexCount = meshopt_generateVertexRemapMulti(remap.data(), 
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

    if (config & CONFIG_FLAG_TRIANGLELIST_PRIMITIVE_TOPOLOGY)
        meshopt_optimizeVertexCache(mesh.faceIndices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount);
    else
        meshopt_optimizeVertexCacheStrip(mesh.faceIndices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount);

    remap.resize(vertexCount);
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

    if ((config & CONFIG_FLAG_TRIANGLELIST_PRIMITIVE_TOPOLOGY) == 0)
    {
        std::vector<uint32_t> indices;
        indices.resize(meshopt_stripifyBound(mesh.faceIndices.size()));
        indices.resize(meshopt_stripify(indices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount, static_cast<uint16_t>(-1)));
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
                vertexElement.format = (config & CONFIG_FLAG_D3D11_VERTEX_FORMAT) ? VertexFormat::DEC3N : VertexFormat::DEC3N_360;
                break;

            case VertexType::TexCoord:
                vertexElement.format = VertexFormat::FLOAT2_HALF;
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
}

static void optimizeMeshGroup(MeshGroup& meshGroup, Config config)
{
    for (auto& mesh : meshGroup.opaqueMeshes)
        optimizeMesh(mesh, config);

    for (auto& mesh : meshGroup.transparentMeshes)
        optimizeMesh(mesh, config);

    for (auto& mesh : meshGroup.punchThroughMeshes)
        optimizeMesh(mesh, config);

    for (auto& group : meshGroup.specialMeshGroups | std::views::values)
    {
        for (auto& mesh : group)
            optimizeMesh(mesh, config);
    }
}

void ModelOptimizer::optimize(Model& model, Config config)
{
    for (auto& meshGroup : model.meshGroups)
        optimizeMeshGroup(meshGroup, config);

}