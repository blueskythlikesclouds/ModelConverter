#include "Parameter.h"

#include "SampleChunk.h"

template <typename T>
void Parameter<T>::write(SampleChunk& out) const
{
    out.write(static_cast<uint32_t>(values.size() << 8));
    out.writeOffset(1, [&]
    {
        out.write(name); 
    });
    out.writeOffset(4, [&]
    {
        for (const auto& value : values)
            out.write(value);
    });
}

template struct Parameter<Float4>;
template struct Parameter<Int4>;
template struct Parameter<BOOL>;