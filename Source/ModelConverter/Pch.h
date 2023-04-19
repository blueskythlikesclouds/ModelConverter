#pragma once

#include <Windows.h>

#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meshoptimizer.h>

#include <mikktspace.h>

using Float3 = std::array<float, 3>;
using Float4 = std::array<float, 4>;
using Int4 = std::array<int32_t, 4>;
using UInt4 = std::array<uint32_t, 4>;

struct Vector4
{
    union
    {
        Float4 f;
        Int4 i;
        UInt4 u;

        struct
        {
            float fx;
            float fy;
            float fz;
            float fw;
        };

        struct
        {
            int32_t ix;
            int32_t iy;
            int32_t iz;
            int32_t iw;
        };

        struct
        {
            uint32_t ux;
            uint32_t uy;
            uint32_t uz;
            uint32_t uw;
        };
    };

    Vector4()
        : fx(), fy(), fz(), fw()
    {
    }

    Vector4(float x)
        : fx(x), fy(), fz(), fw()
    {
    }

    Vector4(float x, float y)
        : fx(x), fy(y), fz(), fw()
    {
    }

    Vector4(float x, float y, float z)
        : fx(x), fy(y), fz(z), fw()
    {
    }

    Vector4(float x, float y, float z, float w)
        : fx(x), fy(y), fz(z), fw(w)
    {
    }

    Vector4(int32_t x)
        : ix(x), iy(), iz(), iw()
    {
    }

    Vector4(int32_t x, int32_t y)
        : ix(x), iy(y), iz(), iw()
    {
    }

    Vector4(int32_t x, int32_t y, int32_t z)
        : ix(x), iy(y), iz(z), iw()
    {
    }

    Vector4(int32_t x, int32_t y, int32_t z, int32_t w)
        : ix(x), iy(y), iz(z), iw(w)
    {
    }

    Vector4(uint32_t x)
        : ux(x), uy(), uz(), uw()
    {
    }

    Vector4(uint32_t x, uint32_t y)
        : ux(x), uy(y), uz(), uw()
    {
    }

    Vector4(uint32_t x, uint32_t y, uint32_t z)
        : ux(x), uy(y), uz(z), uw()
    {
    }

    Vector4(uint32_t x, uint32_t y, uint32_t z, uint32_t w)
        : ux(x), uy(y), uz(z), uw(w)
    {
    }
};

using Float4x4 = std::array<float, 16>;