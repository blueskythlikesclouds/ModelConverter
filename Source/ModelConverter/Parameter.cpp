#include "Parameter.h"

#include "SampleChunk.h"

template <typename T>
void Parameter<T>::write(SampleChunk& out) const
{
    out.write(static_cast<uint32_t>(values.size() << 8));
    out.writeOffset([&]
    {
        out.write(name); 
    });
    out.writeOffset([&]
    {
        for (const auto& value : values)
            out.write(value);
    });
}

template<> void Parameter<Float4>::write(SampleChunk& out) const;
template<> void Parameter<Int4>::write(SampleChunk& out) const;
template<> void Parameter<BOOL>::write(SampleChunk& out) const;