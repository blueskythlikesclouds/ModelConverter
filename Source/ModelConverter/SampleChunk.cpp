#include "SampleChunk.h"

struct SampleChunk::OffsetWrite
{
    size_t alignment;
    std::function<void()> function;
    size_t offsetPosition;
    size_t offset;

    OffsetWrite(size_t alignment, std::function<void()>&& function, size_t offsetPosition)
        : alignment(alignment)
        , function(std::move(function))
        , offsetPosition(offsetPosition)
        , offset()
    {
    }

    void write(SampleChunk& sampleChunk)
    {
        sampleChunk.align(alignment);
        offset = sampleChunk.position;
        function();
    }
};

SampleChunk::SampleChunk()
    : position()
    , beginOffset()
    , dataVersion()
    , dataOffset()
{
}

SampleChunk::~SampleChunk() = default;

void SampleChunk::write(const void* value, size_t size)
{
    if (position + size > data.size())
        data.resize(position + size);

    memcpy(&data[position], value, size);
    position += size;
}

void SampleChunk::write(const std::string& value)
{
    write(value.data(), value.size() + 1);
}

void SampleChunk::write(const char* value)
{
    write(value, strlen(value) + 1);
}

void SampleChunk::writeOffset(size_t alignment, std::function<void()>&& function)
{
    offsetWrites.emplace_back(alignment, std::move(function), position);
    write<uint32_t>(0);
}

void SampleChunk::align(size_t alignment)
{
    const size_t aligned = (position + alignment - 1) & ~(alignment - 1);

    if (data.size() < aligned)
        data.resize(aligned);

    position = aligned;
}

void SampleChunk::writeNulls(size_t count)
{
    while (count > 0)
    {
        write<uint8_t>(0);
        --count;
    }
}

void SampleChunk::begin(uint32_t version)
{
    beginOffset = position;
    dataVersion = version;
    writeNulls(24);
    dataOffset = position;
}

void SampleChunk::end()
{
    for (auto& offsetWrite : offsetWrites)
        offsetWrite.write(*this);

    align(4);
    const size_t relocationOffset = position;

    for (const auto& offsetWrite : offsetWrites)
    {
        position = offsetWrite.offsetPosition;
        write(static_cast<uint32_t>(offsetWrite.offset - dataOffset));
    }

    position = relocationOffset;

    write(static_cast<uint32_t>(offsetWrites.size()));

    for (const auto& offsetWrite : offsetWrites)
        write(static_cast<uint32_t>(offsetWrite.offsetPosition - dataOffset));

    offsetWrites.clear();

    const size_t endOffset = position;
    position = beginOffset;

    write(static_cast<uint32_t>(endOffset - beginOffset));
    write(dataVersion);
    write(static_cast<uint32_t>(relocationOffset - dataOffset));
    write(static_cast<uint32_t>(dataOffset - beginOffset));
    write(static_cast<uint32_t>(relocationOffset - beginOffset));
    write<uint32_t>(0);

    position = endOffset;
}

void SampleChunk::save(const char* path) const
{
    FILE* file = fopen(path, "wb");
    fwrite(data.data(), 1, data.size(), file);
    fclose(file);
}
