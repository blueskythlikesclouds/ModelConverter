#include "OffsetWrite.h"

#include "SampleChunkWriter.h"

OffsetWrite::OffsetWrite(size_t priority, size_t alignment, std::function<void()>&& function, size_t offsetPosition)
    : priority(priority)
    , alignment(alignment)
    , function(std::move(function))
    , offsetPosition(offsetPosition)
{
}

size_t OffsetWrite::write(SampleChunkWriter& writer) const
{
    writer.align(alignment);
    const size_t offset = writer.currentOffset;
    function();
    return offset;
}
