#pragma once

struct SampleChunkWriter;

enum class VertexFormat : uint32_t
{
    FLOAT1 = 0x2C83A4,
    FLOAT2 = 0x2C23A5,
    FLOAT3 = 0x2A23B9,
    FLOAT4 = 0x1A23A6,
    INT1 = 0x2C83A1,
    INT2 = 0x2C23A2,
    INT4 = 0x1A23A3,
    UINT1 = 0x2C82A1,
    UINT2 = 0x2C22A2,
    UINT4 = 0x1A22A3,
    INT1N = 0x2C81A1,
    INT2N = 0x2C21A2,
    INT4N = 0x1A21A3,
    UINT1N = 0x2C80A1,
    UINT2N = 0x2C20A2,
    UINT4N = 0x1A20A3,
    D3DCOLOR = 0x182886,
    UBYTE4 = 0x1A2286,
    BYTE4 = 0x1A2386,
    UBYTE4N = 0x1A2086,
    BYTE4N = 0x1A2186,
    SHORT2 = 0x2C2359,
    SHORT4 = 0x1A235A,
    USHORT2 = 0x2C2259,
    USHORT4 = 0x1A225A,
    SHORT2N = 0x2C2159,
    SHORT4N = 0x1A215A,
    USHORT2N = 0x2C2059,
    USHORT4N = 0x1A205A,
    UDEC3 = 0x2A2287,
    DEC3 = 0x2A2387,
    UDEC3N = 0x2A2087,
    DEC3N = 0x2A2187,
    UDEC4 = 0x1A2287,
    DEC4 = 0x1A2387,
    UDEC4N = 0x1A2087,
    DEC4N = 0x1A2187,
    UHEND3 = 0x2A2290,
    HEND3 = 0x2A2390,
    UHEND3N = 0x2A2090,
    HEND3N = 0x2A2190,
    UDHEN3 = 0x2A2291,
    DHEN3 = 0x2A2391,
    UDHEN3N = 0x2A2091,
    DHEN3N = 0x2A2191,
    FLOAT16_2 = 0x2C235F,
    FLOAT16_4 = 0x1A2360,
    UNUSED = 0xFFFFFFFF
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

    void write(SampleChunkWriter& writer) const;
    void write(SampleChunkWriter& writer, const Vector4& value) const;
};
