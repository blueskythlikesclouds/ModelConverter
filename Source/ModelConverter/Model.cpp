#include "Model.h"

#include "MeshGroup.h"
#include "Node.h"
#include "SampleChunk.h"

Model::Model()
    : min({+INFINITY, +INFINITY, +INFINITY})
    , max({-INFINITY, -INFINITY, -INFINITY})
{
    
}
Model::~Model() = default;

void Model::write(SampleChunk& out) const
{
    out.write(static_cast<uint32_t>(meshGroups.size()));
    out.writeOffset(4, [&]
    {
        for (const auto& meshGroup : meshGroups)
        {
            out.writeOffset(4, [&]
            {
                meshGroup.write(out);
            });
        }
    });

    out.write<uint32_t>(0);
    out.writeOffset(4, [&]
    {
    });

    out.write<uint32_t>(static_cast<uint32_t>(nodes.size()));
    out.writeOffset(4, [&]
    {
        for (const auto& node : nodes)
        {
            out.writeOffset(4, [&]
            {
                node.write(out);
            });
        }
    });
    out.writeOffset(4, [&]
    {
        for (const auto& node : nodes)
            out.write(node.matrix);
    });

    out.writeOffset(4, [&]
    {
        out.write(min[0]);
        out.write(max[0]);
        out.write(min[1]);
        out.write(max[1]);
        out.write(min[2]);
        out.write(max[2]);
    });
}

void Model::save(const char* path) const
{
    SampleChunk out;
    out.begin(nodes.size() > 256 ? 6 : 5);
    write(out);
    out.end();
    out.save(path);
}
