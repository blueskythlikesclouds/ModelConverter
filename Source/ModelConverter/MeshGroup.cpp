#include "MeshGroup.h"

#include "Mesh.h"
#include "SampleChunkWriter.h"

static void writeMeshes(SampleChunkWriter& writer, uint32_t dataVersion, const std::vector<Mesh>& meshes)
{
    writer.write(static_cast<uint32_t>(meshes.size()));
    writer.writeOffset(4, [&]
    {
       for (const auto& mesh : meshes)
       {
           writer.writeOffset(4, [&]
           {
               mesh.write(writer, dataVersion);
           });
       }
    });
}

MeshGroup::MeshGroup() = default;
MeshGroup::~MeshGroup() = default;

void MeshGroup::write(SampleChunkWriter& writer, uint32_t dataVersion) const
{
    writeMeshes(writer, dataVersion, opaqueMeshes);
    writeMeshes(writer, dataVersion, transparentMeshes);
    writeMeshes(writer, dataVersion, punchThroughMeshes);

    writer.write(static_cast<uint32_t>(specialMeshGroups.size()));
    if (!specialMeshGroups.empty())
    {
        writer.writeOffset(4, [&]
        {
            for (const auto& type : specialMeshGroups | std::views::keys)
            {
                writer.writeOffset(1, [&]
                {
                    writer.write(type);
                });
            }
        });
        writer.writeOffset(4, [&]
        {
           for (auto& group : specialMeshGroups | std::views::values)
           {
               writer.writeOffset(4, [&]
               {
                   writer.write(static_cast<uint32_t>(group.size()));
               });
           }
        });
        writer.writeOffset(4, [&, dataVersion]
        {
            for (auto& group : specialMeshGroups | std::views::values)
            {
                writer.writeOffset(4, [&, dataVersion]
                {
                    for (auto& mesh : group)
                    {
                        writer.writeOffset(4, [&, dataVersion]
                        {
                            mesh.write(writer, dataVersion);
                        });
                    }
                });
            }
        });
    }
    else
    {
        writer.write<uint32_t>(~0);
        writer.write<uint32_t>(~0);
        writer.write<uint32_t>(~0);
    }

    writer.write(name);
    writer.align(4);
}
