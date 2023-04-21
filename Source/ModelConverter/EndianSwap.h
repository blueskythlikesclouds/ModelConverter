#pragma once

template<typename T>
static T endianSwap(const T& value)
{
    return value;
}

static int16_t endianSwap(const int16_t& value)
{
    return static_cast<int16_t>(_byteswap_ushort(static_cast<uint16_t>(value)));
}

static uint16_t endianSwap(const uint16_t& value)
{
    return _byteswap_ushort(value);
}

static int32_t endianSwap(const int32_t& value)
{
    return static_cast<int32_t>(_byteswap_ulong(static_cast<uint32_t>(value)));
}

static uint32_t endianSwap(const uint32_t& value)
{
    return _byteswap_ulong(value);
}

static uint32_t endianSwap(const float& value)
{
    union
    {
        float f;
        uint32_t i;
    } u;

    u.f = value;
    return _byteswap_ulong(u.i);
}

static Int4 endianSwap(const Int4& value)
{
    return
    {
        endianSwap(value[0]),
        endianSwap(value[1]),
        endianSwap(value[2]),
        endianSwap(value[3])
    };
}

static UInt3 endianSwap(const Float3& value)
{
    return
    {
        endianSwap(value[0]),
        endianSwap(value[1]),
        endianSwap(value[2])
    };
}

static UInt4 endianSwap(const Float4& value)
{
    return
    {
        endianSwap(value[0]),
        endianSwap(value[1]),
        endianSwap(value[2]),
        endianSwap(value[3])
    };
}

static UInt4x4 endianSwap(const Float4x4& value)
{
    return
    {
        endianSwap(value[0]), endianSwap(value[1]), endianSwap(value[2]), endianSwap(value[3]),
        endianSwap(value[4]), endianSwap(value[5]), endianSwap(value[6]), endianSwap(value[7]),
        endianSwap(value[8]), endianSwap(value[9]), endianSwap(value[10]), endianSwap(value[11]),
        endianSwap(value[12]), endianSwap(value[13]), endianSwap(value[14]), endianSwap(value[15]),
    };
}