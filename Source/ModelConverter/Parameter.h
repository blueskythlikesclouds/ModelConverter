﻿#pragma once

struct SampleChunk;

template<typename T>
struct Parameter
{
    std::string name;
    std::vector<T> values;

    Parameter()
    {
    }

    Parameter(std::string name, std::initializer_list<T> values)
        : name(std::move(name)), values(values)
    {
    }

    void write(SampleChunk& out) const;
};