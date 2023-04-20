#include "VertexElement.h"

#include "SampleChunkWriter.h"

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

void VertexElement::write(SampleChunkWriter& writer) const
{
    writer.write<uint16_t>(stream);
    writer.write<uint16_t>(offset);
    writer.write(static_cast<uint32_t>(format));
    writer.write(static_cast<uint8_t>(method));
    writer.write(static_cast<uint8_t>(type));
    writer.write(index);
    writer.write<uint8_t>(0);
}

void VertexElement::write(SampleChunkWriter& writer, const Vector4& value) const
{
    switch (format)
    {
    case VertexFormat::FLOAT1:
        writer.write(value.f[0]);
        break;

    case VertexFormat::FLOAT2:
        writer.write(value.f[0]);
        writer.write(value.f[1]);
        break;

    case VertexFormat::FLOAT3:
        writer.write(value.f[0]);
        writer.write(value.f[1]);
        writer.write(value.f[2]);
        break;

    case VertexFormat::FLOAT4:
        writer.write(value.f[0]);
        writer.write(value.f[1]);
        writer.write(value.f[2]);
        writer.write(value.f[3]);
        break;

    case VertexFormat::D3DCOLOR:
        writer.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[0], 8)));
        writer.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[1], 8)));
        writer.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[2], 8)));
        writer.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[3], 8)));
        break;

    case VertexFormat::UBYTE4:
    case VertexFormat::UBYTE4_2:
        writer.write(static_cast<uint8_t>(value.u[0]));
        writer.write(static_cast<uint8_t>(value.u[1]));
        writer.write(static_cast<uint8_t>(value.u[2]));
        writer.write(static_cast<uint8_t>(value.u[3]));
        break;

    case VertexFormat::SHORT2:
        writer.write(static_cast<int16_t>(value.i[0]));
        writer.write(static_cast<int16_t>(value.i[1]));
        break;

    case VertexFormat::SHORT4:
        writer.write(static_cast<int16_t>(value.i[0]));
        writer.write(static_cast<int16_t>(value.i[1]));
        writer.write(static_cast<int16_t>(value.i[2]));
        writer.write(static_cast<int16_t>(value.i[3]));
        break;

    case VertexFormat::UBYTE4N:
    case VertexFormat::UBYTE4N_2:
        writer.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[0], 8)));
        writer.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[1], 8)));
        writer.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[2], 8)));
        writer.write(static_cast<uint8_t>(meshopt_quantizeUnorm(value.f[3], 8)));
        break;

    case VertexFormat::SHORT2N:
        writer.write(static_cast<int16_t>(meshopt_quantizeSnorm(value.f[0], 16)));
        writer.write(static_cast<int16_t>(meshopt_quantizeSnorm(value.f[1], 16)));
        break;

    case VertexFormat::SHORT4N:
        writer.write(static_cast<uint16_t>(meshopt_quantizeSnorm(value.f[0], 16)));
        writer.write(static_cast<uint16_t>(meshopt_quantizeSnorm(value.f[1], 16)));
        writer.write(static_cast<uint16_t>(meshopt_quantizeSnorm(value.f[2], 16)));
        writer.write(static_cast<uint16_t>(meshopt_quantizeSnorm(value.f[3], 16)));
        break;

    case VertexFormat::USHORT2N:
        writer.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[0], 16)));
        writer.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[1], 16)));
        break;

    case VertexFormat::USHORT4N:
        writer.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[0], 16)));
        writer.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[1], 16)));
        writer.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[2], 16)));
        writer.write(static_cast<uint16_t>(meshopt_quantizeUnorm(value.f[3], 16)));
        break;

    case VertexFormat::UDEC3:
        writer.write((value.u[0] & 0x3FF) | ((value.u[1] & 0x3FF) << 10) | ((value.u[2] & 0x3FF) << 20));
        break;

    case VertexFormat::DEC3N:
        writer.write(static_cast<uint32_t>(
            (meshopt_quantizeSnorm(value.f[0], 10) & 0x3FF) |
            ((meshopt_quantizeSnorm(value.f[1], 10) & 0x3FF) << 10) |
            ((meshopt_quantizeSnorm(value.f[2], 10) & 0x3FF) << 20)));
        break;

    case VertexFormat::DEC3N_360:
        writer.write(static_cast<uint32_t>(
            ((meshopt_quantizeSnorm(value.f[0], 9) & 0x1FF) << 2) |
            ((meshopt_quantizeSnorm(value.f[1], 9) & 0x1FF) << 13) |
            ((meshopt_quantizeSnorm(value.f[2], 9) & 0x1FF) << 23)));
        break;

    case VertexFormat::FLOAT2_HALF:
        writer.write(meshopt_quantizeHalf(value.f[0]));
        writer.write(meshopt_quantizeHalf(value.f[1]));
        break;

    case VertexFormat::FLOAT4_HALF:
        writer.write(meshopt_quantizeHalf(value.f[0]));
        writer.write(meshopt_quantizeHalf(value.f[1]));
        writer.write(meshopt_quantizeHalf(value.f[2]));
        writer.write(meshopt_quantizeHalf(value.f[3]));
        break;

    case VertexFormat::USHORT4:
        writer.write(static_cast<uint16_t>(value.u[0]));
        writer.write(static_cast<uint16_t>(value.u[1]));
        writer.write(static_cast<uint16_t>(value.u[2]));
        writer.write(static_cast<uint16_t>(value.u[3]));
        break;
    }
}
