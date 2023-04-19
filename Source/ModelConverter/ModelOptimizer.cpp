#include "ModelOptimizer.h"

#include "Mesh.h"
#include "MeshGroup.h"
#include "Model.h"

static void optimizeMesh(Mesh& mesh)
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
    meshopt_optimizeVertexCache(mesh.faceIndices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount);

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

    std::vector<uint16_t> indices;
    indices.resize(meshopt_stripifyBound(mesh.faceIndices.size()));
    indices.resize(meshopt_stripify(indices.data(), mesh.faceIndices.data(), mesh.faceIndices.size(), vertexCount, static_cast<uint16_t>(-1)));
    std::swap(mesh.faceIndices, indices);
}

static void optimizeMeshGroup(MeshGroup& meshGroup)
{
    for (auto& mesh : meshGroup.opaqueMeshes)
        optimizeMesh(mesh);

    for (auto& mesh : meshGroup.transparentMeshes)
        optimizeMesh(mesh);

    for (auto& mesh : meshGroup.punchThroughMeshes)
        optimizeMesh(mesh);

    for (auto& group : meshGroup.specialMeshGroups)
    {
        for (auto& mesh : group)
            optimizeMesh(mesh);
    }
}

void ModelOptimizer::optimize(Model& model)
{
    for (auto& meshGroup : model.meshGroups)
        optimizeMeshGroup(meshGroup);
}
