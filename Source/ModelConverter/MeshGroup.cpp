#include "MeshGroup.h"

#include "Mesh.h"
#include "SampleChunk.h"

static void writeMeshes(SampleChunk& out, const std::vector<Mesh>& meshes)
{
    out.write(static_cast<uint32_t>(meshes.size()));
    out.writeOffset(4, [&]
    {
       for (const auto& mesh : meshes)
       {
           out.writeOffset(4, [&]
           {
               mesh.write(out);
           });
       }
    });
}

MeshGroup::MeshGroup() = default;
MeshGroup::~MeshGroup() = default;

void MeshGroup::write(SampleChunk& out) const
{
    writeMeshes(out, opaqueMeshes);
    writeMeshes(out, transparentMeshes);
    writeMeshes(out, punchThroughMeshes);

    out.write(static_cast<uint32_t>(specialMeshGroups.size()));
    if (!specialMeshGroups.empty())
    {
        out.writeOffset(4, [&]
        {
            for (const auto& type : specialMeshGroups | std::views::keys)
            {
                out.writeOffset(1, [&]
                {
                    out.write(type);
                });
            }
        });
        out.writeOffset(4, [&]
        {
           for (auto& group : specialMeshGroups | std::views::values)
           {
               out.writeOffset(4, [&]
               {
                   out.write(static_cast<uint32_t>(group.size()));
               });
           }
        });
        out.writeOffset(4, [&]
        {
            for (auto& group : specialMeshGroups | std::views::values)
            {
                out.writeOffset(4, [&]
                {
                    for (auto& mesh : group)
                    {
                        out.writeOffset(4, [&]
                        {
                            mesh.write(out);
                        });
                    }
                });
            }
        });
    }
    else
    {
        out.write<uint32_t>(~0);
        out.write<uint32_t>(~0);
        out.write<uint32_t>(~0);
    }

    out.write(name);
    out.align(4);
}
