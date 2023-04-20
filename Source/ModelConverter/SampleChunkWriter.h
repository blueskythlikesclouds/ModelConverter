#pragma once

#include "EndianSwap.h"

struct SampleChunkNode;
struct OffsetWrite;

struct SampleChunkWriter
{
    std::vector<uint8_t> data;
    size_t currentOffset;
    std::list<OffsetWrite> offsetWrites;
    std::vector<std::pair<size_t, size_t>> offsets;

    SampleChunkWriter();
    ~SampleChunkWriter();

    void write(const void* value, size_t size);

    template<typename T>
    void write(const T& value)
    {
        const T bigEndianValue = endianSwap(value);
        write(&bigEndianValue, sizeof(T));
    }

    void write(const std::string& value);
    void write(const char* value);

    void writeOffset(size_t priority, size_t alignment, std::function<void()>&& function);

    template<typename T>
    void writeOffset(size_t priority, size_t alignment, const T& function)
    {
        writeOffset(priority, alignment, std::function<void()>(function));
    }

    template<typename T>
    void writeOffset(size_t alignment, const T& function)
    {
        writeOffset(0, alignment, function);
    }

    void align(size_t alignment);
    void writeNulls(size_t count);

    void flush();

    static SampleChunkWriter write(uint32_t dataVersion, std::function<void(SampleChunkWriter&)> function);
    static SampleChunkWriter write(const SampleChunkNode& node);

    bool save(const char* path) const;
};
