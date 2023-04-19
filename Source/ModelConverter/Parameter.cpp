#include "Parameter.h"

#include "SampleChunkWriter.h"

template <typename T>
void Parameter<T>::write(SampleChunkWriter& writer) const
{
    writer.write(static_cast<uint32_t>(values.size() << 8));
    writer.writeOffset(1, [&]
    {
        writer.write(name); 
    });
    writer.writeOffset(4, [&]
    {
        for (const auto& value : values)
            writer.write(value);
    });
}

template struct Parameter<Float4>;
template struct Parameter<Int4>;
template struct Parameter<BOOL>;