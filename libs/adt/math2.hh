#pragma once

#include "mathDecl2.hh"
#include "print.hh"

namespace adt::math
{

constexpr inline f64 toDeg(f64 x) { return x * 180.0 / PI64; }
constexpr inline f64 toRad(f64 x) { return x * PI64 / 180.0; }
constexpr inline f32 toDeg(f32 x) { return x * 180.0f / PI32; }
constexpr inline f32 toRad(f32 x) { return x * PI32 / 180.0f; }

constexpr inline f64 toRad(i64 x) { return toRad(static_cast<f64>(x)); }
constexpr inline f64 toDeg(i64 x) { return toDeg(static_cast<f64>(x)); }
constexpr inline f32 toRad(i32 x) { return toRad(static_cast<f32>(x)); }
constexpr inline f32 toDeg(i32 x) { return toDeg(static_cast<f32>(x)); }

inline bool
eq(const f64 l, const f64 r)
{
    return std::abs(l - r) <= EPS64*(std::abs(l) + std::abs(r) + 1.0);
}

inline bool
eq(const f32 l, const f32 r)
{
    return std::abs(l - r) <= EPS32*(std::abs(l) + std::abs(r) + 1.0f);
}

constexpr inline auto sq(const auto& x) { return x * x; }
constexpr inline auto cube(const auto& x) { return x*x*x; }

/* V2 */

inline V2::V2() : m_aData {} {}

inline V2::V2(f32 x, f32 y) : m_aData {x, y} {}

inline V2::V2(const f32 (&a)[2]) : m_aData {a[0], a[1]} {}

inline V2::V2(IV2 iv2) : V2{f32(iv2.x()), f32(iv2.y())} {}

inline f32& V2::x() { return m_aData[0]; }
inline const f32& V2::x() const { return m_aData[0]; }

inline f32& V2::y() { return m_aData[1]; }
inline const f32& V2::y() const { return m_aData[1]; }

/* V2 end */

/* IV2 */

inline IV2::IV2() : m_aData {} {}

inline IV2::IV2(i32 x, i32 y) : m_aData {x, y} {}

inline IV2::IV2(const i32 (&a)[2]) : m_aData {a[0], a[1]} {}

inline IV2::IV2(V2 v2) : IV2{i32(v2.x()), i32(v2.y())} {}

inline i32& IV2::x() { return m_aData[0]; }
inline const i32& IV2::x() const { return m_aData[0]; }

inline i32& IV2::y() { return m_aData[1]; }
inline const i32& IV2::y() const { return m_aData[1]; }

/* IV2 end */

/* V3 */

inline V3::V3() : m_v2 {}, m_z {} {}

inline V3::V3(f32 x, f32 y, f32 z) : m_v2 {x, y}, m_z {z} {}

inline V3::V3(V2 v2, f32 z) : m_v2 {v2}, m_z {z} {}

inline V3::V3(const f32 (&a)[3]) : m_v2 {a[0], a[1]}, m_z {a[2]} {}

inline f32& V3::x() { return m_v2.x(); }
inline const f32& V3::x() const { return m_v2.x(); }

inline f32& V3::y() { return m_v2.y(); }
inline const f32& V3::y() const { return m_v2.y(); }

inline f32& V3::z() { return m_z; }
inline const f32& V3::z() const { return m_z; }

inline V2& V3::xy() { return m_v2; }
inline const V2& V3::xy() const { return m_v2; }

/* V3 end */

/* IV3 */

inline IV3::IV3() : m_v2 {}, m_z {} {}
inline IV3::IV3(i32 x, i32 y, i32 z) : m_v2 {x, y}, m_z {z} {}

/* IV3 end */

/* V4 */

inline V4::V4() : m_v3 {}, m_w {} {}
inline V4::V4(f32 x, f32 y, f32 z, f32 w) : m_v3 {x, y, z}, m_w {w} {}
inline V4::V4(V3 v3, f32 w) : m_v3 {v3}, m_w{w} {}

/* V4 end */

/* M3 */

inline M3::M3() : m_v30 {}, m_v31 {}, m_v32 {} {}

inline M3::M3(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8)
    : m_v30 {_0, _1, _2}, m_v31 {_3, _4, _5}, m_v32 {_6, _7, _8} {}

inline M3
M3::iden()
{
    return {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };
}

/* M3 end */

/* M4 */

inline M4::M4() : m_v40 {}, m_v41 {}, m_v42 {}, m_v43 {} {}

inline M4::M4(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8, f32 _9, f32 _10, f32 _11, f32 _12, f32 _13, f32 _14, f32 _15)
    : m_v40 {_0, _1, _2, _3}, m_v41 {_4, _5, _6, _7}, m_v42 {_8, _9, _10, _11}, m_v43 {_12, _13, _14, _15} {}

inline M4::M4(const f32 (&a)[16])
    : M4(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15]) {}

inline M4
M4::iden()
{
    return {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

inline void
M4::print()
{
}

/* M4 end */

} /* namespace adt::math */


namespace adt::print
{

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::V2& x)
{
    return formatToContext(ctx, fmtArgs, x.data());
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::V3& x)
{
    return formatToContext(ctx, fmtArgs, x.data());
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::V4& x)
{
    return formatToContext(ctx, fmtArgs, x.data());
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::M3& x)
{
    auto& e = x.data3x3();

    ctx.fmt = "\n\t[{:.3}, {:.3}, {:.3}"
              "\n\t {:.3}, {:.3}, {:.3}"
              "\n\t {:.3}, {:.3}, {:.3}]";
    ctx.fmtIdx = 0;
    return printArgs(ctx,
        e[0][0], e[0][1], e[0][2],
        e[1][0], e[1][1], e[1][2],
        e[2][0], e[2][1], e[2][2]
    );
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::M4& x)
{
    auto& e = x.data4x4();

    ctx.fmt = "\n\t[{:.3}, {:.3}, {:.3}, {:.3}"
              "\n\t {:.3}, {:.3}, {:.3}, {:.3}"
              "\n\t {:.3}, {:.3}, {:.3}, {:.3}"
              "\n\t {:.3}, {:.3}, {:.3}, {:.3}]";
    ctx.fmtIdx = 0;
    return printArgs(ctx,
        e[0][0], e[0][1], e[0][2], e[0][3],
        e[1][0], e[1][1], e[1][2], e[1][3],
        e[2][0], e[2][1], e[2][2], e[2][3],
        e[3][0], e[3][1], e[3][2], e[3][3]
    );
}

} /* namespace adt::print */
