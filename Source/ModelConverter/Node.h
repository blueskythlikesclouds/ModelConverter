﻿#pragma once

struct SampleChunk;

struct Node
{
    uint32_t parentIndex;
    std::string name;
    Float4x4 matrix;

    void write(SampleChunk& out) const;
};
