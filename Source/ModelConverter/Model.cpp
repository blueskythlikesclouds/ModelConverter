#include "Model.h"

#include "Config.h"
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

    writer.write<uint32_t>(0);
    writer.writeOffset(4, [&]
    {
    });

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

    writer.writeOffset(4, [&]
    {
        writer.write(min[0]);
        writer.write(max[0]);
        writer.write(min[1]);
        writer.write(max[1]);
        writer.write(min[2]);
        writer.write(max[2]);
    });
}

bool Model::save(const char* path, Config config) const
{
    const uint32_t dataVersion = nodes.size() > 255 ? 6 : 5;

    if (config & CONFIG_FLAG_V2_SAMPLE_CHUNK)
    {
        SampleChunkNode model("Model", 1);

        if (!scaParameters.empty())
        {
            auto& nodesExt = model.children.emplace_back("NodesExt", 1);
            const size_t count = !nodes.empty() ? nodes.size() : 1;

            for (size_t i = 0; i < count; i++)
            {
                auto& nodePrms = nodesExt.children.emplace_back("NodePrms", static_cast<int>(i));
                auto& scaParam = nodePrms.children.emplace_back("SCAParam", 1);

                for (const auto& [name, value] : scaParameters)
                    scaParam.children.emplace_back(name, value);
            }
        }

        if (config & CONFIG_FLAG_TRIANGLE_LIST)
        {
            model.children.emplace_back("Topology", 3);
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
