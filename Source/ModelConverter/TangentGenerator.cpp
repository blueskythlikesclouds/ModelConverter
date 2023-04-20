#include "TangentGenerator.h"

#include "Mesh.h"
#include "MeshGroup.h"
#include "Model.h"
#include "VertexElement.h"

struct TangentGenerator::Context
{
    std::vector<size_t> offsets;
    std::vector<Mesh*> meshes;
    size_t indexCount;

    Context() : indexCount()
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
    return static_cast<int>(static_cast<TangentGenerator::Context*>(pContext->m_pUserData)->indexCount / 3);
}

static int getNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace)
{
    return 3;
}

static void getPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
{
    const Vector4& position = 
        static_cast<TangentGenerator::Context*>(pContext->m_pUserData)->get(VertexType::Position, iFace * 3 + iVert);

    fvPosOut[0] = position.fx;
    fvPosOut[1] = position.fy;
    fvPosOut[2] = position.fz;
}

static void getNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
{
    const Vector4& normal = 
        static_cast<TangentGenerator::Context*>(pContext->m_pUserData)->get(VertexType::Normal, iFace * 3 + iVert);

    fvNormOut[0] = normal.fx;
    fvNormOut[1] = normal.fy;
    fvNormOut[2] = normal.fz;
}

static void getTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
{
    const Vector4& texCoord = 
        static_cast<TangentGenerator::Context*>(pContext->m_pUserData)->get(VertexType::TexCoord, iFace * 3 + iVert);

    fvTexcOut[0] = texCoord.fx;
    fvTexcOut[1] = 1.0f - texCoord.fy;
}

static void setTSpace(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT,
	const tbool bIsOrientationPreserving, const int iFace, const int iVert)
{
    Vector4& tangent = 
        static_cast<TangentGenerator::Context*>(pContext->m_pUserData)->get(VertexType::Tangent, iFace * 3 + iVert);

    Vector4& binormal = 
        static_cast<TangentGenerator::Context*>(pContext->m_pUserData)->get(VertexType::Binormal, iFace * 3 + iVert);

    tangent.fx = fvTangent[0];
    tangent.fy = fvTangent[1];
    tangent.fz = fvTangent[2];
    tangent.fw = 0.0f;

    binormal.fx = fvBiTangent[0];
    binormal.fy = fvBiTangent[1];
    binormal.fz = fvBiTangent[2];
    binormal.fw = 0.0f;
}

static SMikkTSpaceInterface interface = { getNumFaces, getNumVerticesOfFace, getPosition, getNormal, getTexCoord, nullptr, setTSpace };

static void addMesh(TangentGenerator::Context& context, Mesh& mesh)
{
    auto& normals = mesh.vertexStreams[static_cast<size_t>(VertexType::Normal)];
    if (!normals.empty() && !normals[0].empty())
    {
        context.offsets.push_back(context.indexCount);
        context.meshes.push_back(&mesh);
        context.indexCount += normals[0].size();
    }
}

static void addMeshGroup(TangentGenerator::Context& context, MeshGroup& meshGroup)
{
    for (auto& mesh : meshGroup.opaqueMeshes)
        addMesh(context, mesh);

    for (auto& mesh : meshGroup.transparentMeshes)
        addMesh(context, mesh);

    for (auto& mesh : meshGroup.punchThroughMeshes)
        addMesh(context, mesh);

    for (auto& group : meshGroup.specialMeshGroups | std::views::values)
    {
        for (auto& mesh : group)
            addMesh(context, mesh);
    }
}

void TangentGenerator::generate(Model& model)
{
    Context context;

    for (auto& meshGroup : model.meshGroups)
        addMeshGroup(context, meshGroup);

    SMikkTSpaceContext ctx = { &interface, &context };
    genTangSpaceDefault(&ctx);
}
