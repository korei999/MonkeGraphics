#pragma once

#include "adt/Span.hh"

#include <immintrin.h>

namespace adt::simd
{

inline void
fillF32(Span<f32> src, const f32 x)
{
    __m128 packX = _mm_set_ps1(x);

    ssize i = 0;
    for (; i + 3 < src.getSize(); i += 4)
        _mm_storeu_ps(&src[i], packX);

    for (; i < src.getSize(); ++i)
        src[i] = x;
}

} /* namespace adt::simd */
