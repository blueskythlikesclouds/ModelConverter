#pragma once

struct SampleChunk;

enum class VertexFormat : uint32_t
{
    FLOAT1 = 0x2C83A4,
    FLOAT2 = 0x2C23A5,
    FLOAT3 = 0x2A23B9,
    FLOAT4 = 0x1A23A6,
    D3DCOLOR = 0x182886,
    UBYTE4 = 0x1A2286,
    UBYTE4_2 = 0x1A2386,
    SHORT2 = 0x2C2359,
    SHORT4 = 0x1A235A,
    UBYTE4N = 0x1A2086,
    UBYTE4N_2 = 0x1A2186,
    SHORT2N = 0x2C2159,
    SHORT4N = 0x1A215A,
    USHORT2N = 0x2C2059,
    USHORT4N = 0x1A205A,
    UDEC3 = 0x2A2287,
    DEC3N = 0x2A2187,
    DEC3N_360 = 0x2A2190,
    FLOAT2_HALF = 0x2C235F,
    FLOAT4_HALF = 0x1A2360,
    USHORT4 = 0x1A225A,
    Unused = 0xFFFFFFFF
};

enum class VertexMethod
{
    Normal,
    PartialU,
    PartialV,
    CrossUV,
    UV,
    Lookup,
    LookupPresampled
};

enum class VertexType
{
    Position,
    BlendWeight,
    BlendIndices,
    Normal,
    PSize,
    TexCoord,
    Tangent,
    Binormal,
    TessFactor,
    PositionT,
    Color,
    Fog,
    Depth,
    Sample
};

struct VertexElement
{
    uint16_t stream;
    uint16_t offset;
    VertexFormat format;
    VertexMethod method;
    VertexType type;
    uint8_t index;

    VertexElement();
    VertexElement(size_t offset, VertexFormat format, VertexType type, size_t index);

    size_t getSize() const;
    size_t getNextOffset() const;

    void write(SampleChunk& out) const;
    void write(SampleChunk& out, const Vector4& value) const;
};
