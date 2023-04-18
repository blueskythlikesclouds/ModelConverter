#pragma once

template<typename T>
struct Parameter;

struct Material
{
    std::string shader;
    uint8_t alphaThreshold;
    bool doubleSided;
    bool additive;
    std::vector<Parameter<Float4>> float4Parameters;
    std::vector<Parameter<Int4>> int4Parameters;
    std::vector<Parameter<BOOL>> boolParameters;
};
