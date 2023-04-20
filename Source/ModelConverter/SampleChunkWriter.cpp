#include "SampleChunkWriter.h"
#include "OffsetWrite.h"
#include "SampleChunkNode.h"

SampleChunkWriter::SampleChunkWriter()
    : currentOffset()
{
}

SampleChunkWriter::~SampleChunkWriter() = default;

void SampleChunkWriter::write(const void* value, size_t size)
{
    if (currentOffset + size > data.size())
        data.resize(currentOffset + size);

    memcpy(&data[currentOffset], value, size);
    currentOffset += size;
}

void SampleChunkWriter::write(const std::string& value)
{
    write(value.data(), value.size() + 1);
}

void SampleChunkWriter::write(const char* value)
{
    write(value, strlen(value) + 1);
}

void SampleChunkWriter::writeOffset(size_t priority, size_t alignment, std::function<void()>&& function)
{
    offsetWrites.emplace_back(priority, alignment, std::move(function), currentOffset);
    write<uint32_t>(0);
}

void SampleChunkWriter::align(size_t alignment)
{
    const size_t aligned = (currentOffset + alignment - 1) & ~(alignment - 1);

    if (data.size() < aligned)
        data.resize(aligned);

    currentOffset = aligned;
}

void SampleChunkWriter::writeNulls(size_t count)
{
    while (count > 0)
    {
        write<uint8_t>(0);
        --count;
    }
}

static void writeOffsets(SampleChunkWriter& writer, std::list<OffsetWrite>::iterator first, std::list<OffsetWrite>::iterator last)
{
    if (first == last)
        return;

    --last;

    bool increment;
    size_t priority = 0;

    do
    {
        increment = false;

        for (auto it = first; it != std::next(last); ++it)
        {
            if (it->priority == priority)
            {
                const auto beforeChild = std::prev(writer.offsetWrites.end());

                writer.offsets.emplace_back(it->offsetPosition, it->write(writer));

                writeOffsets(writer, std::next(beforeChild), writer.offsetWrites.end());
            }
            else if (it->priority > priority)
            {
                increment = true;
            }
        }

        ++priority;
    } while (increment);
}

void SampleChunkWriter::flush()
{
    writeOffsets(*this, offsetWrites.begin(), offsetWrites.end());
    offsetWrites.clear();
}

SampleChunkWriter SampleChunkWriter::write(uint32_t dataVersion, std::function<void(SampleChunkWriter&)> function)
{
    SampleChunkWriter writer;

    const size_t headerOffset = writer.currentOffset;
    writer.writeNulls(24);
    const size_t dataOffset = writer.currentOffset;

    function(writer);
    writer.flush();

    writer.align(4);
    const size_t relocationOffset = writer.currentOffset;

    for (const auto& [offsetPosition, offset] : writer.offsets)
    {
        writer.currentOffset = offsetPosition;
        writer.write(static_cast<uint32_t>(offset - dataOffset));
    }

    writer.currentOffset = relocationOffset;

    writer.write(static_cast<uint32_t>(writer.offsets.size()));

    for (const auto& [offsetPosition, offset] : writer.offsets)
        writer.write(static_cast<uint32_t>(offsetPosition - dataOffset));

    const size_t endOffset = writer.currentOffset;
    writer.currentOffset = headerOffset;

    writer.write(static_cast<uint32_t>(endOffset - headerOffset));
    writer.write(dataVersion);
    writer.write(static_cast<uint32_t>(relocationOffset - dataOffset));
    writer.write(static_cast<uint32_t>(dataOffset - headerOffset));
    writer.write(static_cast<uint32_t>(relocationOffset - headerOffset));
    writer.write<uint32_t>(0);

    writer.currentOffset = endOffset;

    return writer;
}

SampleChunkWriter SampleChunkWriter::write(const SampleChunkNode& node)
{
    SampleChunkWriter writer;

    const size_t headerOffset = writer.currentOffset;
    writer.writeNulls(16);

    const size_t dataOffset = writer.currentOffset;

    node.write(writer, SAMPLE_CHUNK_NODE_FLAG_IS_TAIL);
    writer.align(16);

    const size_t relocationOffset = writer.currentOffset;

    for (const auto& [offsetPosition, offset] : writer.offsets)
    {
        writer.currentOffset = offsetPosition;
        writer.write(static_cast<uint32_t>(offset - dataOffset));
    }

    writer.currentOffset = relocationOffset;

    for (const auto& [offsetPosition, offset] : writer.offsets)
        writer.write(static_cast<uint32_t>(offsetPosition - dataOffset));

    const size_t endOffset = writer.currentOffset;

    writer.currentOffset = headerOffset;

    writer.write(static_cast<uint32_t>(0x80000000 | (endOffset - headerOffset)));
    writer.write<uint32_t>(20120906);
    writer.write(static_cast<uint32_t>(relocationOffset - headerOffset));
    writer.write(static_cast<uint32_t>(writer.offsets.size()));

    writer.currentOffset = endOffset;

    return writer;
}

void SampleChunkWriter::save(const char* path) const
{
    FILE* file = fopen(path, "wb");
    fwrite(data.data(), 1, data.size(), file);
    fclose(file);
}   
