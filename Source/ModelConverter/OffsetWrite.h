#pragma once

struct SampleChunkWriter;

struct OffsetWrite
{
    size_t priority;
    size_t alignment;
    std::function<void()> function;
    size_t offsetPosition;

    OffsetWrite(size_t priority, size_t alignment, std::function<void()>&& function, size_t offsetPosition);

    size_t write(SampleChunkWriter& writer) const;
};
