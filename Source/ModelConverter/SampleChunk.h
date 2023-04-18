#pragma once

#include "EndianSwap.h"

struct SampleChunk
{
    struct OffsetWrite;

    FILE* file;
    std::list<OffsetWrite> offsetWrites;

    long beginOffset;
    uint32_t dataVersion;
    long dataOffset;

    SampleChunk(FILE* file);
    SampleChunk(const char* path);

    ~SampleChunk();

    long tell() const;
    void seek(long offset, int origin) const;

    void write(const void* value, size_t size) const;

    template<typename T>
    void write(const T& value) const
    {
        const T bigEndianValue = endianSwap(value);
        write(&bigEndianValue, sizeof(T));
    }

    void write(const std::string& value) const;
    void write(const char* value) const;

    void writeOffset(long alignment, std::function<void()>&& function);

    template<typename T>
    void writeOffset(long alignment, const T& function)
    {
        writeOffset(alignment, std::function<void()>(function));
    }

    void align(long alignment) const;
    void writeNulls(size_t count) const;

    void begin(uint32_t version);
    void end();

    void close();
};
