#pragma once

#include "EndianSwap.h"

struct SampleChunk
{
    struct OffsetWrite;

    std::vector<uint8_t> data;
    size_t position;
    std::list<OffsetWrite> offsetWrites;

    size_t beginOffset;
    uint32_t dataVersion;
    size_t dataOffset;

    SampleChunk();
    ~SampleChunk();

    void write(const void* value, size_t size);

    template<typename T>
    void write(const T& value)
    {
        const T bigEndianValue = endianSwap(value);
        write(&bigEndianValue, sizeof(T));
    }

    void write(const std::string& value);
    void write(const char* value);

    void writeOffset(size_t alignment, std::function<void()>&& function);

    template<typename T>
    void writeOffset(size_t alignment, const T& function)
    {
        writeOffset(alignment, std::function<void()>(function));
    }

    void align(size_t alignment);
    void writeNulls(size_t count);

    void begin(uint32_t version);
    void end();

    void save(const char* path) const;
};
