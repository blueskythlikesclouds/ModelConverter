#include "Model.h"

#include "Config.h"
#include "Mesh.h"
#include "MeshGroup.h"
#include "Node.h"
#include "SampleChunkNode.h"
#include "SampleChunkWriter.h"

Model::Model()
    : min({+INFINITY, +INFINITY, +INFINITY})
    , max({-INFINITY, -INFINITY, -INFINITY})
{
    
}

Model::Model(const Model&) = default;
Model::Model(Model&&) noexcept = default;
Model::~Model() = default;

void Model::write(SampleChunkWriter& writer, uint32_t dataVersion) const
{
    if (dataVersion >= 5)
    {
        writer.write(static_cast<uint32_t>(meshGroups.size()));
        writer.writeOffset(4, [&, dataVersion]
        {
            for (const auto& meshGroup : meshGroups)
            {
                writer.writeOffset(4, [&, dataVersion]
                {
                    meshGroup.write(writer, dataVersion);
                });
            }
        });
    }
    else
    {
        size_t opaqMeshCount = 0;
        size_t transMeshCount = 0;
        size_t punchMeshCount = 0;

        for (const auto& meshGroup : meshGroups)
        {
            opaqMeshCount += meshGroup.opaqueMeshes.size();
            transMeshCount += meshGroup.transparentMeshes.size();
            punchMeshCount += meshGroup.punchThroughMeshes.size();
        }

        writer.write(static_cast<uint32_t>(opaqMeshCount));
        writer.writeOffset(4, [&, dataVersion]
        {
            for (auto& meshGroup : meshGroups)
            {
               for (const auto& mesh : meshGroup.opaqueMeshes)
               {
                   writer.writeOffset(4, [&, dataVersion]
                   {
                       mesh.write(writer, dataVersion);
                   });
               }
            }
        });
        writer.write(static_cast<uint32_t>(transMeshCount));
        writer.writeOffset(4, [&, dataVersion]
        {
            for (auto& meshGroup : meshGroups)
            {
               for (const auto& mesh : meshGroup.transparentMeshes)
               {
                   writer.writeOffset(4, [&, dataVersion]
                   {
                       mesh.write(writer, dataVersion);
                   });
               }
            }
        });
        writer.write(static_cast<uint32_t>(punchMeshCount));
        writer.writeOffset(4, [&, dataVersion]
        {
            for (auto& meshGroup : meshGroups)
            {
               for (const auto& mesh : meshGroup.punchThroughMeshes)
               {
                   writer.writeOffset(4, [&, dataVersion]
                   {
                       mesh.write(writer, dataVersion);
                   });
               }
            }
        });
    }

    if (dataVersion >= 4)
    {
        writer.write<uint32_t>(0);
        writer.writeOffset(4, [&]
        {
        });
    }

    writer.write(static_cast<uint32_t>(nodes.size()));
    writer.writeOffset(4, [&]
    {
        for (const auto& node : nodes)
        {
            writer.writeOffset(4, [&]
            {
                node.write(writer);
            });
        }
    });
    writer.writeOffset(4, [&]
    {
        for (const auto& node : nodes)
            writer.write(node.matrix);
    });

    if (dataVersion >= 2)
    {
        writer.writeOffset(4, [&]
        {
            writer.write(min[0]);
            writer.write(max[0]);
            writer.write(min[1]);
            writer.write(max[1]);
            writer.write(min[2]);
            writer.write(max[2]);
        });

        if (dataVersion == 2)
        {
            writer.writeOffset(4, [&]
            {
            });
        }
    }
}

bool Model::save(const char* path, Config config) const
{
    uint32_t dataVersion;

    if (config & CONFIG_FLAG_V4_MODEL)
        dataVersion = 4;

    else if (nodes.size() > 255)
        dataVersion = 6;

    else
        dataVersion = 5;

    if (config & CONFIG_FLAG_V2_SAMPLE_CHUNK)
    {
        SampleChunkNode model("Model", 1);

        if (!scaParameters.empty())
        {
            auto& nodesExt = model.children.emplace_back("NodesExt", 1);
            const size_t count = !meshGroups.empty() ? meshGroups.size() : 1;

            for (size_t i = 0; i < count; i++)
            {
                auto& nodePrms = nodesExt.children.emplace_back("NodePrms", static_cast<int>(i));
                auto& scaParam = nodePrms.children.emplace_back("SCAParam", 1);

                for (const auto& [name, value] : scaParameters)
                    scaParam.children.emplace_back(name, value);
            }
        }

        if (config & CONFIG_FLAG_RAYTRACING_VERTEX_FORMAT)
            model.children.emplace_back("GensRT  ", true);

        if (config & CONFIG_FLAG_TRIANGLE_LIST)
        {
            model.children.emplace_back("Topology", 3);

            if (config & CONFIG_FLAG_D3D11_VERTEX_FORMAT)
                model.children.emplace_back("UserAABB", false);
        }

        model.children.emplace_back("Contexts", dataVersion, [&, dataVersion](SampleChunkWriter& writer)
        {
            write(writer, dataVersion);
        });

        return SampleChunkWriter::write(model).save(path);
    }
    else
    {
        return SampleChunkWriter::write(dataVersion, [&, dataVersion](SampleChunkWriter& writer)
        {
            write(writer, dataVersion);
        }).save(path);
    }
}
