/* 
 * <mmintrin.h>  MMX
 * <xmmintrin.h> SSE
 * <emmintrin.h> SSE2
 * <pmmintrin.h> SSE3
 * <tmmintrin.h> SSSE3
 * <smmintrin.h> SSE4.1
 * <nmmintrin.h> SSE4.2
 * <ammintrin.h> SSE4A
 * <wmmintrin.h> AES
 * <immintrin.h> AVX, AVX2, FMA
 */

#pragma once

#include "adt/Span.hh"
#include "adt/math.hh"

#include <immintrin.h>

namespace adt::simd
{

/* 128 bit */

struct f32x4;

struct i32x4
{
    __m128i pack {};

    /* */

    i32x4() = default;

    i32x4(const __m128i _pack) : pack(_pack) {}

    i32x4(const i32 x) : pack(_mm_set1_epi32(x)) {}

    i32x4(const i32 x, const i32 y, const i32 z, const i32 w) : pack(_mm_set_epi32(w, z, y, x)) {}

    /* */

    explicit operator __m128i() const { return pack; }

    explicit operator f32x4() const;

    /* */

    i32* data() { return reinterpret_cast<i32*>(this); }
    const i32* data() const { return (i32*)(this); }

    i32& operator[](int i)             { ADT_ASSERT(i >= 0 && i < 4, "out of range, should be (>= 0 && < 4)"); return data()[i]; }
    const i32& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 4, "out of range, should be (>= 0 && < 4)"); return data()[i]; }
};

struct f32x4
{
    __m128 pack {};

    /* */

    f32x4() = default;

    f32x4(const __m128 _pack) : pack(_pack) {}

    f32x4(const f32 x) : pack(_mm_set1_ps(x)) {}

    f32x4(f32 x, f32 y, f32 z, f32 w) : pack(_mm_set_ps(w, z, y, x)) {}

    f32x4(const math::V4 v) : pack(_mm_set_ps(v.w, v.z, v.y, v.x)) {}

    /* */

    explicit operator __m128() const { return pack; }

    explicit operator i32x4() const { return _mm_cvtps_epi32(pack); }
};

struct V2x4
{
    f32x4 x {}, y {};

    /* */

    f32x4* data() { return reinterpret_cast<f32x4*>(this); }
    const f32x4* data() const { return (f32x4*)(this); }

    f32x4& operator[](int i)             { ADT_ASSERT(i >= 0 && i < 2, "out of range"); return data()[i]; }
    const f32x4& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 2, "out of range"); return data()[i]; }
};

struct IV2x4
{
    i32x4 x {}, y {};

    /* */

    i32x4* data() { return reinterpret_cast<i32x4*>(this); }
    const i32x4* data() const { return (i32x4*)(this); }

    i32x4& operator[](int i)             { ADT_ASSERT(i >= 0 && i < 2, "out of range"); return data()[i]; }
    const i32x4& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 2, "out of range"); return data()[i]; }
};

inline f32x4
floor(const f32x4 x)
{
    return _mm_floor_ps(x.pack);
}

inline i32x4
i32x4Reinterpret(const f32x4 x)
{
    return _mm_castps_si128(x.pack);
}

inline f32x4
f32x4Reinterpret(const i32x4 x)
{
    return _mm_castsi128_ps(x.pack);
}

inline
i32x4::operator f32x4() const
{
    return _mm_cvtepi32_ps(pack);
}

inline i32x4
i32x4Load(i32* const ptr)
{
    return _mm_loadu_si128(reinterpret_cast<__m128i*>(ptr));
}

inline i32x4
i32x4Gather(i32* const p, const i32x4 offSets)
{
    return {p[ offSets[0] ], p[ offSets[1] ], p[ offSets[2] ], p[ offSets[3] ]};
}

inline f32x4
f32x4Load(const f32* const ptr)
{
    return _mm_loadu_ps(ptr);
}

inline void
f32x4Store(f32* const pDest, const f32x4 x)
{
    _mm_storeu_ps(pDest, x.pack);
}

inline void
i32x4Store(i32* const pDest, const i32x4 x)
{
    _mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), x.pack);
}

inline i32x4
operator+(const i32x4 l, const i32x4 r)
{
    return _mm_add_epi32(l.pack, r.pack);
}

inline f32x4
operator+(const f32x4 l, const f32x4 r)
{
    return _mm_add_ps(l.pack, r.pack);
}

inline f32x4
operator-(const f32x4 l, const f32x4 r)
{
    return _mm_sub_ps(l.pack, r.pack);
}

inline f32x4&
operator+=(f32x4& l, const f32x4 r)
{
    return l = l + r;
}

inline f32x4&
operator-=(f32x4& l, const f32x4 r)
{
    return l = l - r;
}

inline i32x4
operator-(const i32x4 l, const i32x4 r)
{
    return _mm_sub_epi32(l.pack, r.pack);
}

inline i32x4&
operator+=(i32x4& l, const i32x4 r)
{
    return l = l + r;
}

inline i32x4&
operator-=(i32x4& l, const i32x4 r)
{
    return l = l - r;
}

inline i32x4
operator*(const i32x4 l, const i32x4 r)
{
    return _mm_mullo_epi32(l.pack, r.pack);
}

inline i32x4&
operator*=(i32x4& l, const i32x4 r)
{
    return l = l * r;
}

inline f32x4
operator*(const f32x4 l, const f32x4 r)
{
    return _mm_mul_ps(l.pack, r.pack);
}

inline f32x4
operator*(const f32x4 l, const f32 r)
{
    return l * f32x4(r);
}

inline f32x4
operator/(const f32x4 l, const f32x4 r)
{
    return _mm_div_ps(l.pack, r.pack);
}

inline i32x4
operator|(const i32x4 l, const i32x4 r)
{
    return _mm_or_si128(l.pack, r.pack);
}

inline i32x4
operator&(const i32x4 l, const i32x4 r)
{
    return _mm_and_si128(l.pack, r.pack);
}

inline f32x4
operator&(const f32x4 l, const f32x4 r)
{
    return _mm_and_ps(l.pack, r.pack);
}

inline f32x4
operator-(const f32x4 r)
{
    return r * f32x4(-1.0f);
}

inline f32x4
operator<(const f32x4 l, const f32x4 r)
{
    return _mm_cmplt_ps(l.pack, r.pack);
}

inline i32x4
operator>=(const i32x4 l, const i32x4 r)
{
    return _mm_cmpgt_epi32(l.pack, r.pack) | _mm_cmpeq_epi32(l.pack, r.pack);
}

inline i32x4
operator<(const i32x4 l, const i32x4 r)
{
    return _mm_cmplt_epi32(l.pack, r.pack);
}

inline V2x4
operator+(const V2x4 l, const V2x4 r)
{
    return {
        .x = l.x + r.x,
        .y = l.y + r.y,
    };
}

inline V2x4
operator-(const V2x4 l, const V2x4 r)
{
    return {
        .x = l.x - r.x,
        .y = l.y - r.y,
    };
}

inline V2x4
operator-(const V2x4 l, const math::V2 r)
{
    return {
        .x = l.x - r.x,
        .y = l.y - r.y,
    };
}

inline V2x4
operator+(const V2x4 l, const math::V2 r)
{
    return {
        .x = l.x + r.x,
        .y = l.y + r.y,
    };
}

inline V2x4
operator*(const f32x4 l, math::V2 r)
{
    return {
        .x = l * r.x,
        .y = l * r.y,
    };
}

inline V2x4
operator*(const V2x4 l, math::V2 r)
{
    return {
        .x = l.x * r.x,
        .y = l.y * r.y,
    };
}

inline V2x4
operator/(const V2x4 l, const f32x4 r)
{
    return {
        .x = l.x / r,
        .y = l.y / r,
    };
}

inline V2x4&
operator/=(V2x4& l, const f32x4 r)
{
    return l = l / r;
}

inline i32x4
andNot(const i32x4 mask, i32x4 x)
{
    return _mm_andnot_si128(mask.pack, x.pack);
}

inline f32x4
andNot(const f32x4 mask, f32x4 x)
{
    return _mm_andnot_ps(mask.pack, x.pack);
}

inline int
moveMask8(const i32x4 x)
{
    return _mm_movemask_epi8(x.pack);
}

inline i32x4
min(const i32x4 l, const i32x4 r)
{
    return _mm_min_epi32(l.pack, r.pack);
}

inline i32x4
max(const i32x4 l, const i32x4 r)
{
    return _mm_max_epi32(l.pack, r.pack);
}

inline void
i32Fillx4(Span<i32> src, const i32 x)
{
    const i32 pack = x;

    ssize i = 0;
    for (; i + 3 < src.getSize(); i += 4)
        i32x4Store(&src[i], pack);

    for (; i < src.getSize(); ++i)
        src[i] = x;
}

inline void
f32Fillx4(Span<f32> src, const f32 x)
{
    f32x4 pack = x;

    ssize i = 0;
    for (; i + 3 < src.getSize(); i += 4)
        f32x4Store(&src[i], pack);

    for (; i < src.getSize(); ++i)
        src[i] = x;
}

/* 128 bit end */

#if defined ADT_AVX2

struct f32x8;

struct i32x8
{
    __m256i pack {};

    /* */

    i32x8() = default;

    i32x8(const __m256i _pack) : pack(_pack) {}

    i32x8(const i32 x) : pack(_mm256_set1_epi32(x)) {}

    i32x8(i32 x0, i32 x1, i32 x2, i32 x3, i32 x4, i32 x5, i32 x6, i32 x7)
        : pack(_mm256_set_epi32(x7, x6, x5, x4, x3, x2, x1, x0)) {}

    /* */

    explicit operator __m256i() const { return pack; }

    explicit operator f32x8() const;

    /* */

    i32* data() { return reinterpret_cast<i32*>(this); }
    const i32* data() const { return (i32*)(this); }

    i32& operator[](int i)             { ADT_ASSERT(i >= 0 && i < 8, "out of range, should be (>= 0 && < 8)"); return data()[i]; }
    const i32& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 8, "out of range, should be (>= 0 && < 8)"); return data()[i]; }
};

struct f32x8
{
    __m256 pack {};

    /* */

    f32x8() = default;

    f32x8(const __m256 _pack) : pack(_pack) {}

    f32x8(const f32 x) : pack(_mm256_set1_ps(x)) {}

    /* */

    explicit operator __m256() const { return pack; }

    explicit operator i32x8() const { return _mm256_cvtps_epi32(pack); }
};

struct V2x8
{
    f32x8 x {}, y {};

    /* */

    f32x8* data() { return reinterpret_cast<f32x8*>(this); }
    const f32x8* data() const { return (f32x8*)(this); }

    f32x8& operator[](int i)             { ADT_ASSERT(i >= 0 && i < 2, "out of range"); return data()[i]; }
    const f32x8& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 2, "out of range"); return data()[i]; }
};

struct IV2x8
{
    i32x8 x {}, y {};

    /* */

    i32x8* data() { return reinterpret_cast<i32x8*>(this); }
    const i32x8* data() const { return (i32x8*)(this); }

    i32x8& operator[](int i)             { ADT_ASSERT(i >= 0 && i < 2, "out of range"); return data()[i]; }
    const i32x8& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 2, "out of range"); return data()[i]; }
};

inline f32x8
floor(const f32x8 x)
{
    return _mm256_floor_ps(x.pack);
}

inline i32x8
i32x8Reinterpret(const f32x8 x)
{
    return _mm256_castps_si256(x.pack);
}

inline f32x8
f32x8Reinterpret(const i32x8 x)
{
    return _mm256_castsi256_ps(x.pack);
}

inline
i32x8::operator f32x8() const
{
    return _mm256_cvtepi32_ps(pack);
}

inline i32x8
i32x8Load(i32* const ptr)
{
    return _mm256_loadu_si256(reinterpret_cast<__m256i*>(ptr));
}

inline i32x8
i32x8Gather(i32* const p, const i32x8 offSets)
{
    return {
        p[ offSets[0] ], p[ offSets[1] ], p[ offSets[2] ], p[ offSets[3] ],
        p[ offSets[4] ], p[ offSets[5] ], p[ offSets[6] ], p[ offSets[7] ],
    };
}

inline f32x8
f32x8Load(const f32* const ptr)
{
    return _mm256_loadu_ps(ptr);
}

inline void
f32x8Store(f32* const pDest, const f32x8 x)
{
    _mm256_storeu_ps(pDest, x.pack);
}

inline void
i32x8Store(i32* const pDest, const i32x8 x)
{
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(pDest), x.pack);
}

inline i32x8
operator+(const i32x8 l, const i32x8 r)
{
    return _mm256_add_epi32(l.pack, r.pack);
}

inline f32x8
operator+(const f32x8 l, const f32x8 r)
{
    return _mm256_add_ps(l.pack, r.pack);
}

inline f32x8
operator-(const f32x8 l, const f32x8 r)
{
    return _mm256_sub_ps(l.pack, r.pack);
}

inline f32x8&
operator+=(f32x8& l, const f32x8 r)
{
    return l = l + r;
}

inline f32x8&
operator-=(f32x8& l, const f32x8 r)
{
    return l = l - r;
}

inline i32x8
operator-(const i32x8 l, const i32x8 r)
{
    return _mm256_sub_epi32(l.pack, r.pack);
}

inline i32x8&
operator+=(i32x8& l, const i32x8 r)
{
    return l = l + r;
}

inline i32x8&
operator-=(i32x8& l, const i32x8 r)
{
    return l = l - r;
}

inline i32x8
operator*(const i32x8 l, const i32x8 r)
{
    return _mm256_mullo_epi32(l.pack, r.pack);
}

inline i32x8&
operator*=(i32x8& l, const i32x8 r)
{
    return l = l * r;
}

inline f32x8
operator*(const f32x8 l, const f32x8 r)
{
    return _mm256_mul_ps(l.pack, r.pack);
}

inline f32x8
operator*(const f32x8 l, const f32 r)
{
    return l * f32x8(r);
}

inline f32x8
operator/(const f32x8 l, const f32x8 r)
{
    return _mm256_div_ps(l.pack, r.pack);
}

inline i32x8
operator|(const i32x8 l, const i32x8 r)
{
    return _mm256_or_si256(l.pack, r.pack);
}

inline i32x8
operator&(const i32x8 l, const i32x8 r)
{
    return _mm256_and_si256(l.pack, r.pack);
}

inline f32x8
operator&(const f32x8 l, const f32x8 r)
{
    return _mm256_and_ps(l.pack, r.pack);
}

inline f32x8
operator-(const f32x8 r)
{
    return r * f32x8(-1.0f);
}

inline f32x8
operator<(const f32x8 l, const f32x8 r)
{
    return _mm256_cmp_ps(l.pack, r.pack, _CMP_LT_OQ);
}

inline i32x8
operator>=(const i32x8 l, const i32x8 r)
{
    return _mm256_cmpgt_epi32(l.pack, r.pack) | _mm256_cmpeq_epi32(l.pack, r.pack);
}

inline i32x8
operator<(const i32x8 l, const i32x8 r)
{
    /* in reverse */
    return _mm256_cmpgt_epi32(r.pack, l.pack);
}

inline V2x8
operator+(const V2x8 l, const V2x8 r)
{
    return {
        .x = l.x + r.x,
        .y = l.y + r.y,
    };
}

inline V2x8
operator-(const V2x8 l, const V2x8 r)
{
    return {
        .x = l.x - r.x,
        .y = l.y - r.y,
    };
}

inline V2x8
operator-(const V2x8 l, const math::V2 r)
{
    return {
        .x = l.x - r.x,
        .y = l.y - r.y,
    };
}

inline V2x8
operator+(const V2x8 l, const math::V2 r)
{
    return {
        .x = l.x + r.x,
        .y = l.y + r.y,
    };
}

inline V2x8
operator*(const f32x8 l, math::V2 r)
{
    return {
        .x = l * r.x,
        .y = l * r.y,
    };
}

inline V2x8
operator*(const V2x8 l, math::V2 r)
{
    return {
        .x = l.x * r.x,
        .y = l.y * r.y,
    };
}

inline V2x8
operator/(const V2x8 l, const f32x8 r)
{
    return {
        .x = l.x / r,
        .y = l.y / r,
    };
}

inline V2x8&
operator/=(V2x8& l, const f32x8 r)
{
    return l = l / r;
}

inline i32x8
andNot(const i32x8 mask, i32x8 x)
{
    return _mm256_andnot_si256(mask.pack, x.pack);
}

inline f32x8
andNot(const f32x8 mask, f32x8 x)
{
    return _mm256_andnot_ps(mask.pack, x.pack);
}

inline int
moveMask8(const i32x8 x)
{
    return _mm256_movemask_epi8(x.pack);
}

inline i32x8
min(const i32x8 l, const i32x8 r)
{
    return _mm256_min_epi32(l.pack, r.pack);
}

inline i32x8
max(const i32x8 l, const i32x8 r)
{
    return _mm256_max_epi32(l.pack, r.pack);
}

inline void
i32Fillx8(Span<i32> src, const i32 x)
{
    const i32 pack = x;

    ssize i = 0;
    for (; i + 7 < src.getSize(); i += 8)
        i32x8Store(&src[i], pack);

    for (; i < src.getSize(); ++i)
        src[i] = x;
}

inline void
f32Fillx8(Span<f32> src, const f32 x)
{
    f32x8 pack = x;

    ssize i = 0;
    for (; i + 7 < src.getSize(); i += 8)
        f32x8Store(&src[i], pack);

    for (; i < src.getSize(); ++i)
        src[i] = x;
}

#endif /* ADT_AVX2 */

} /* namespace adt::simd */
