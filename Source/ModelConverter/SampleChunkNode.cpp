#include "SampleChunkNode.h"

#include "SampleChunkWriter.h"

SampleChunkNode::SampleChunkNode()
    : value()
{
}

SampleChunkNode::SampleChunkNode(std::string name, int value)
    : name(std::move(name))
    , value(value)
{
}

SampleChunkNode::SampleChunkNode(std::string name, int value, std::function<void(SampleChunkWriter&)>&& function)
    : name(std::move(name))
    , value(value)
    , function(std::move(function))
{
}

void SampleChunkNode::write(SampleChunkWriter& writer, SampleChunkNodeFlags flags) const
{
    const size_t offset = writer.currentOffset;
    writer.write<uint32_t>(0);
    writer.write<int>(value);

    if (name.size() > 8)
        writer.write(name.data(), 8);
    else
    {
        writer.write(name.data(), name.size());
        for (size_t i = name.size(); i < 8; i++)
            writer.write(' ');
    }

    if (!children.empty())
    {
        for (size_t i = 0; i < children.size(); i++)
        {
            children[i].write(writer, i == (children.size() - 1) ? SAMPLE_CHUNK_NODE_FLAG_IS_TAIL : SAMPLE_CHUNK_NODE_FLAG_NONE);
            writer.align(16);
        }
    }
    else
    {
        flags |= SAMPLE_CHUNK_NODE_FLAG_IS_LEAF;
    }

    if (function)
    {
        function(writer);
        writer.flush();
    }

    const size_t endOffset = writer.currentOffset;

    writer.currentOffset = offset;
    writer.write(static_cast<uint32_t>(flags | (endOffset - offset)));

    writer.currentOffset = endOffset;
}
