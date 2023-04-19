#include "Node.h"

#include "SampleChunkWriter.h"

void Node::write(SampleChunkWriter& writer) const
{
    writer.write<uint32_t>(parentIndex);
    writer.writeOffset(1, [&]
    {
        writer.write(name);
    });
}
