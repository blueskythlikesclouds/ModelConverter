#pragma once

struct SampleChunk;

template<typename T>
struct Parameter
{
    std::string name;
    std::vector<T> values;

    void write(SampleChunk& out) const;
};