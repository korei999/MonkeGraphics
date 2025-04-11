#pragma once

#include "types.hh"

#include <limits>

namespace adt::math
{

constexpr f64 PI64 = 3.14159265358979323846;
constexpr f32 PI32 = static_cast<f32>(PI64);
constexpr f64 EPS64 = std::numeric_limits<f64>::epsilon();
constexpr f32 EPS32 = std::numeric_limits<f32>::epsilon();

constexpr inline f64 toDeg(f64 x);
constexpr inline f64 toRad(f64 x);
constexpr inline f32 toDeg(f32 x);
constexpr inline f32 toRad(f32 x);

constexpr inline f64 toRad(i64 x);
constexpr inline f64 toDeg(i64 x);
constexpr inline f32 toRad(i32 x);
constexpr inline f32 toDeg(i32 x);

inline bool eq(const f64 l, const f64 r);
inline bool eq(const f32 l, const f32 r);

constexpr inline auto sq(const auto& x);
constexpr inline auto cube(const auto& x);

struct IV2;

struct V2
{
    f32 m_aData[2];

    /* */

    V2();
    V2(f32 x, f32 y);
    V2(const f32 (&a)[2]);
    V2(IV2 iv2);

    /* */

    f32 (&data())[2] { return *reinterpret_cast<f32(*)[2]>(m_aData); }
    const f32 (&data() const)[2] { return *reinterpret_cast<const f32(*)[2]>(m_aData); }

    f32& x();
    const f32& x() const;

    f32& y();
    const f32& y() const;
};

struct IV2
{
    i32 m_aData[2];

    /* */

    IV2();
    IV2(i32 x, i32 y);
    IV2(const i32 (&a)[2]);
    IV2(V2 v2);

    /* */

    i32 (&data())[2] { return *reinterpret_cast<i32(*)[2]>(this); }
    const i32 (&data() const)[2] { return *reinterpret_cast<const i32(*)[2]>(this); }

    i32& x();
    const i32& x() const;

    i32& y();
    const i32& y() const;
};

struct V3
{
    V2 m_v2;
    f32 m_z;

    /* */

    V3();
    V3(f32 x, f32 y, f32 z);
    V3(V2 v2, f32 z);
    V3(const f32 (&a)[3]);

    /* */

    f32 (&data())[3] { return *reinterpret_cast<f32(*)[3]>(this); }
    const f32 (&data() const)[3] { return *reinterpret_cast<const f32(*)[3]>(this); }

    f32& x();
    const f32& x() const;

    f32& y();
    const f32& y() const;

    f32& z();
    const f32& z() const;

    V2& xy();
    const V2& xy() const;
};

struct IV3
{
    IV2 m_v2;
    i32 m_z;

    /* */

    IV3();
    IV3(i32 x, i32 y, i32 z);

    /* */

    i32 (&data())[3] { return *reinterpret_cast<i32(*)[3]>(this); }
    const i32 (&data() const)[3] { return *reinterpret_cast<const i32(*)[3]>(this); }
};

struct V4
{
    V3 m_v3;
    f32 m_w;

    /* */

    V4();
    V4(f32 x, f32 y, f32 z, f32 w);
    V4(V3 v3, f32 w);

    /* */

    f32 (&data())[4] { return *reinterpret_cast<f32(*)[4]>(this); }
    const f32 (&data() const)[4] { return *reinterpret_cast<const f32(*)[4]>(this); }
};

struct M3
{
    V3 m_v30;
    V3 m_v31;
    V3 m_v32;

    /* */

    M3();
    M3(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8);

    /* */

    static M3 iden();

    /* */

    f32 (&data())[9] { return *reinterpret_cast<f32(*)[9]>(this); }
    const f32 (&data() const)[9] { return *reinterpret_cast<const f32(*)[9]>(this); }

    f32 (&data3x3())[3][3] { return *reinterpret_cast<f32(*)[3][3]>(this); }
    const f32 (&data3x3() const)[3][3] { return *reinterpret_cast<const f32(*)[3][3]>(this); }

    V3 (&dataV3())[3] { return *reinterpret_cast<V3(*)[3]>(this); }
    const V3 (&dataV3() const)[3] { return *reinterpret_cast<const V3(*)[3]>(this); }
};

struct M4
{
    V4 m_v40;
    V4 m_v41;
    V4 m_v42;
    V4 m_v43;

    /* */

    M4();
    M4(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8, f32 _9, f32 _10, f32 _11, f32 _12, f32 _13, f32 _14, f32 _15);
    M4(const f32 (&a)[16]);

    /* */

    static M4 iden();

    /* */

    f32 (&data())[16] { return *reinterpret_cast<f32(*)[16]>(this); }
    const f32 (&data() const)[16] { return *reinterpret_cast<const f32(*)[16]>(this); }

    f32 (&data4x4())[4][4] { return *reinterpret_cast<f32(*)[4][4]>(this); }
    const f32 (&data4x4() const)[4][4] { return *reinterpret_cast<const f32(*)[4][4]>(this); }

    V4 (&dataV4())[4] { return *reinterpret_cast<V4(*)[4]>(this); }
    const V4 (&dataV4() const)[4] { return *reinterpret_cast<const V4(*)[4]>(this); }

    void print();
};

} /* namespace adt::math */
