#pragma once
#include "SampleChunkWriter.h"

struct SampleChunkWriter;

enum SampleChunkNodeFlags
{
    SAMPLE_CHUNK_NODE_FLAG_NONE    = 0 << 0,
    SAMPLE_CHUNK_NODE_FLAG_IS_ROOT = 1 << 31,
    SAMPLE_CHUNK_NODE_FLAG_IS_TAIL = 1 << 30,
    SAMPLE_CHUNK_NODE_FLAG_IS_LEAF = 1 << 29,
};
DEFINE_ENUM_FLAG_OPERATORS(SampleChunkNodeFlags);

struct SampleChunkNode
{
    std::string name;
    int value;
    std::function<void(SampleChunkWriter&)> function;
    std::vector<SampleChunkNode> children;

    SampleChunkNode();
    SampleChunkNode(std::string name, int value);
    SampleChunkNode(std::string name, int value, std::function<void(SampleChunkWriter&)>&& function);

    void write(SampleChunkWriter& writer, SampleChunkNodeFlags flags) const;
};
