#include "VertexElement.h"

#include "SampleChunk.h"

VertexElement::VertexElement()
    : stream(0xFF)
    , offset()
    , format(VertexFormat::Unused)
    , method()
    , type()
    , index()
{
}

VertexElement::VertexElement(size_t offset, VertexFormat format, VertexType type, size_t index)
    : stream()
    , offset(static_cast<uint16_t>(offset))
    , format(format)
    , method()
    , type(type)
    , index(static_cast<uint8_t>(index))
{
}

size_t VertexElement::getSize() const
{
    switch (format)
    {
    case VertexFormat::FLOAT1: return 4;
    case VertexFormat::FLOAT2: return 8;
    case VertexFormat::FLOAT3: return 12;
    case VertexFormat::FLOAT4: return 16;
    case VertexFormat::D3DCOLOR: return 4;
    case VertexFormat::UBYTE4: return 4;
    case VertexFormat::UBYTE4_2: return 4;
    case VertexFormat::SHORT2: return 4;
    case VertexFormat::SHORT4: return 8;
    case VertexFormat::UBYTE4N: return 4;
    case VertexFormat::UBYTE4N_2: return 4;
    case VertexFormat::SHORT2N: return 4;
    case VertexFormat::SHORT4N: return 8;
    case VertexFormat::USHORT2N: return 4;
    case VertexFormat::USHORT4N: return 8;
    case VertexFormat::UDEC3: return 4;
    case VertexFormat::DEC3N: return 4;
    case VertexFormat::DEC3N_360: return 4;
    case VertexFormat::FLOAT2_HALF: return 4;
    case VertexFormat::FLOAT4_HALF: return 8;
    case VertexFormat::USHORT4: return 8;
    }

    return 0;
}

size_t VertexElement::getNextOffset() const
{
    return (offset + getSize() + 0x3) & ~0x3;
}

void VertexElement::write(SampleChunk& out) const
{
    out.write<uint16_t>(stream);
    out.write<uint16_t>(offset);
    out.write(static_cast<uint32_t>(format));
    out.write(static_cast<uint8_t>(method));
    out.write(static_cast<uint8_t>(type));
    out.write(index);
    out.write<uint8_t>(0);
}

void VertexElement::write(SampleChunk& out, const Vector4& value) const
{
    switch (format)
    {
    case VertexFormat::FLOAT1:
        out.write(value.f[0]);
        break;

    case VertexFormat::FLOAT2:
        out.write(value.f[0]);
        out.write(value.f[1]);
        break;

    case VertexFormat::FLOAT3:
        out.write(value.f[0]);
        out.write(value.f[1]);
        out.write(value.f[2]);
        break;

    case VertexFormat::FLOAT4:
        out.write(value.f[0]);
        out.write(value.f[1]);
        out.write(value.f[2]);
        out.write(value.f[3]);
        break;

    case VertexFormat::D3DCOLOR:
        out.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[0], 8)));
        out.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[1], 8)));
        out.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[2], 8)));
        out.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[3], 8)));
        break;

    case VertexFormat::UBYTE4:
    case VertexFormat::UBYTE4_2:
        out.write(static_cast<uint8_t>(value.u[0]));
        out.write(static_cast<uint8_t>(value.u[1]));
        out.write(static_cast<uint8_t>(value.u[2]));
        out.write(static_cast<uint8_t>(value.u[3]));
        break;

    case VertexFormat::SHORT2:
        out.write(static_cast<int16_t>(value.i[0]));
        out.write(static_cast<int16_t>(value.i[1]));
        break;

    case VertexFormat::SHORT4:
        out.write(static_cast<int16_t>(value.i[0]));
        out.write(static_cast<int16_t>(value.i[1]));
        out.write(static_cast<int16_t>(value.i[2]));
        out.write(static_cast<int16_t>(value.i[3]));
        break;

    case VertexFormat::UBYTE4N:
    case VertexFormat::UBYTE4N_2:
        out.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[0], 8)));
        out.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[1], 8)));
        out.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[2], 8)));
        out.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[3], 8)));
        break;

    case VertexFormat::SHORT2N:
        out.write(static_cast<int16_t>(meshopt_quantizeSnorm(value.f[0], 16)));
        out.write(static_cast<int16_t>(meshopt_quantizeSnorm(value.f[1], 16)));
        break;

    case VertexFormat::SHORT4N:
        out.write(static_cast<uint16_t>(meshopt_quantizeSnorm(value.f[0], 16)));
        out.write(static_cast<uint16_t>(meshopt_quantizeSnorm(value.f[1], 16)));
        out.write(static_cast<uint16_t>(meshopt_quantizeSnorm(value.f[2], 16)));
        out.write(static_cast<uint16_t>(meshopt_quantizeSnorm(value.f[3], 16)));
        break;

    case VertexFormat::USHORT2N:
        out.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[0], 16)));
        out.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[1], 16)));
        break;

    case VertexFormat::USHORT4N:
        out.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[0], 16)));
        out.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[1], 16)));
        out.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[2], 16)));
        out.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[3], 16)));
        break;

    case VertexFormat::UDEC3:
        out.write(value.u[0] | value.u[1] << 10 | value.u[2] << 20);
        break;

    case VertexFormat::DEC3N:
        out.write(static_cast<uint32_t>(
            meshopt_quantizeSnorm(value.f[0], 10) |
            (meshopt_quantizeSnorm(value.f[1], 10) << 10) |
            (meshopt_quantizeSnorm(value.f[2], 10) << 20)));
        break;

    case VertexFormat::DEC3N_360:
        out.write(static_cast<uint32_t>(
            (meshopt_quantizeSnorm(value.f[0], 9) << 2) |
            (meshopt_quantizeSnorm(value.f[1], 9) << 13) |
            (meshopt_quantizeSnorm(value.f[2], 9) << 23)));
        break;

    case VertexFormat::FLOAT2_HALF:
        out.write(meshopt_quantizeHalf(value.f[0]));
        out.write(meshopt_quantizeHalf(value.f[1]));
        break;

    case VertexFormat::FLOAT4_HALF:
        out.write(meshopt_quantizeHalf(value.f[0]));
        out.write(meshopt_quantizeHalf(value.f[1]));
        out.write(meshopt_quantizeHalf(value.f[2]));
        out.write(meshopt_quantizeHalf(value.f[3]));
        break;

    case VertexFormat::USHORT4:
        out.write(static_cast<uint16_t>(value.u[0]));
        out.write(static_cast<uint16_t>(value.u[1]));
        out.write(static_cast<uint16_t>(value.u[2]));
        out.write(static_cast<uint16_t>(value.u[3]));
        break;
    }
}
