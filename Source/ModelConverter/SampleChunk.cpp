#include "SampleChunk.h"

struct SampleChunk::OffsetWrite
{
    long alignment;
    std::function<void()> function;
    long offsetPosition;
    long offset;

    OffsetWrite(long alignment, std::function<void()>&& function, long offsetPosition)
        : alignment(alignment)
        , function(std::move(function))
        , offsetPosition(offsetPosition)
        , offset()
    {
    }

    void write(const SampleChunk& sampleChunk)
    {
        sampleChunk.align(alignment);
        offset = ftell(sampleChunk.file);
        function();
    }
};

SampleChunk::SampleChunk(FILE* file)
    : file(file)
    , beginOffset()
    , dataVersion()
    , dataOffset()
{
}

SampleChunk::SampleChunk(const char* path)
    : SampleChunk(fopen(path, "wb"))
{
}

SampleChunk::~SampleChunk()
{
    assert(offsetWrites.empty());

    if (file)
        fclose(file);
}

long SampleChunk::tell() const
{
    return ftell(file);
}

void SampleChunk::seek(long offset, int origin) const
{
    fseek(file, offset, origin);
}

void SampleChunk::write(const void* value, size_t size) const
{
    fwrite(value, 1, size, file);
}

void SampleChunk::write(const std::string& value) const
{
    write(value.data(), value.size() + 1);
}

void SampleChunk::write(const char* value) const
{
    write(value, strlen(value) + 1);
}

void SampleChunk::writeOffset(long alignment, std::function<void()>&& function)
{
    offsetWrites.emplace_back(alignment, std::move(function), ftell(file));
    write<uint32_t>(0);
}

void SampleChunk::align(long alignment) const
{
    long position = ftell(file);
    const long aligned = (position + alignment - 1) & ~(alignment - 1);

    while (position < aligned)
    {
        write<uint8_t>(0);
        ++position;
    }
}

void SampleChunk::writeNulls(size_t count) const
{
    while (count > 0)
    {
        write<uint8_t>(0);
        --count;
    }
}

void SampleChunk::begin(uint32_t version)
{
    beginOffset = ftell(file);
    dataVersion = version;
    writeNulls(24);
    dataOffset = ftell(file);
}

void SampleChunk::end()
{
    for (auto& offsetWrite : offsetWrites)
        offsetWrite.write(*this);

    align(4);
    const long relocationOffset = ftell(file);

    for (const auto& offsetWrite : offsetWrites)
    {
        fseek(file, offsetWrite.offsetPosition, SEEK_SET);
        write(static_cast<uint32_t>(offsetWrite.offset - dataOffset));
    }

    fseek(file, relocationOffset, SEEK_SET);

    write(static_cast<uint32_t>(offsetWrites.size()));

    for (const auto& offsetWrite : offsetWrites)
        write(static_cast<uint32_t>(offsetWrite.offsetPosition - dataOffset));

    offsetWrites.clear();

    const long endOffset = ftell(file);

    fseek(file, beginOffset, SEEK_SET);

    write(static_cast<uint32_t>(endOffset - beginOffset));
    write(dataVersion);
    write(static_cast<uint32_t>(relocationOffset - dataOffset));
    write(static_cast<uint32_t>(dataOffset - beginOffset));
    write(static_cast<uint32_t>(relocationOffset - beginOffset));
    write<uint32_t>(0);

    fseek(file, endOffset, SEEK_SET);
}

void SampleChunk::close()
{
    if (file)
    {
        fclose(file);
        file = nullptr;
    }
}
