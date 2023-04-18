#include "Node.h"

#include "SampleChunk.h"

void Node::write(SampleChunk& out) const
{
    out.write<uint32_t>(parentIndex);
    out.writeOffset(1, [&]
    {
        out.write(name);
    });
}
