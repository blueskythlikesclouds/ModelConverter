#pragma once

struct SampleChunkWriter;

struct Node
{
    uint32_t parentIndex;
    std::string name;
    Float4x4 matrix;

    void write(SampleChunkWriter& writer) const;
};
