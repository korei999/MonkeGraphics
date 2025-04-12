#pragma once

#include "print.hh"

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

template<typename T> struct V3Base;

template<typename T = f32>
struct V2Base
{
    T m_aData[2];

    /* */

    constexpr V2Base() = default;
    constexpr V2Base(T x, T y);
    constexpr V2Base(const T (&a)[2]);
    template<typename Y> constexpr explicit V2Base(V2Base<Y> v2);
    template<typename Y> constexpr explicit V2Base(V3Base<Y> v3);

    /* */

    T (&data())[2] { return *reinterpret_cast<T(*)[2]>(this); }
    const T (&data() const)[2] { return *reinterpret_cast<const T(*)[2]>(this); }

    T& x() { return data()[0]; }
    const T& x() const { return data()[0]; }

    T& y() { return data()[1]; }
    const T& y() const { return data()[1]; }

    T& r() { return x(); }
    const T& r() const { return x(); }

    T& g() { return y(); }
    const T& g() const { return y(); }

    T& operator[](int i) { ADT_ASSERT(i >= 0 && i < 2, "out of range i: {}", i); return *(reinterpret_cast<T*>(this) + i); }
    const T& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 2, "out of range i: {}", i); return *(reinterpret_cast<const T*>(this) + i); }

    V2Base operator-() const;

    V2Base operator+(const V2Base b) const;
    V2Base operator+(T b) const;
    V2Base operator-(const V2Base b) const;
    V2Base operator-(T b) const;
    V2Base operator*(const V2Base b) const;
    V2Base operator*(T b) const;
    V2Base operator/(const V2Base b) const;
    V2Base operator/(T b) const;

    V2Base& operator+=(const V2Base b);
    V2Base& operator+=(T b);
    V2Base& operator-=(const V2Base b);
    V2Base& operator-=(T b);
    V2Base& operator*=(const V2Base b);
    V2Base& operator*=(T b);
    V2Base& operator/=(const V2Base b);
};

template<typename T = f32>
struct V3Base : V2Base<T> /* Does the trick. */
{
    T m_z;

    /* */

    constexpr V3Base() = default;
    constexpr V3Base(const T (&a)[3]);
    constexpr V3Base(T x, T y, T z);

    /* */

    T (&data())[3] { return *reinterpret_cast<T(*)[3]>(this); }
    const T (&data() const)[3] { return *reinterpret_cast<const T(*)[3]>(this); }

    T& z() { return m_z; }
    const T& z() const { return m_z; }

    T& b() { return z(); }
    const T& b() const { return z(); }

    V2Base<T>& xy() { return static_cast<V2Base<T>&>(*this); }

    T& operator[](int i) { ADT_ASSERT(i >= 0 && i < 3, "out of range i: {}", i); return *(reinterpret_cast<T*>(this) + i); }
    const T& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 3, "out of range i: {}", i); return *(reinterpret_cast<T*>(this) + i); }
};

template<typename T>
inline V3Base<T> operator-(const V3Base<T>& a);

template<typename T>
inline V3Base<T> operator+(const V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline V3Base<T>& operator+=(V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline V3Base<T> operator+(const V3Base<T>& a, T b);

template<typename T>
inline V3Base<T>& operator+=(V3Base<T>& a, T b);

template<typename T>
inline V3Base<T> operator-(const V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline V3Base<T>& operator-=(V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline V3Base<T>& operator-=(V3Base<T>& a, T b);

template<typename T>
inline V3Base<T> operator-(const V3Base<T>& a, T b);

template<typename T>
inline V3Base<T> operator*(const V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline V3Base<T>& operator*=(V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline V3Base<T> operator*(const V3Base<T>& a, T b);

template<typename T>
inline V3Base<T> operator*(T a, const V3Base<T>& b);

template<typename T>
inline V3Base<T>& operator*=(V3Base<T>& a, T b);

template<typename T>
inline V3Base<T> operator/(const V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline V3Base<T>& operator/=(V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline V3Base<T> operator/(const V3Base<T>& a, T b);

template<typename T>
inline V3Base<T>& operator/=(V3Base<T>& a, T b);

template<typename T> struct M4Base;

template<typename T>
struct V4Base : V3Base<T>
{
    T m_w;
    
    /* */

    constexpr V4Base() = default;
    constexpr V4Base(const T (&a)[4]) : V3Base<T> {a[0], a[1], a[2]}, m_w {a[3]} {}
    constexpr V4Base(T x, T y, T z, T w) : V3Base<T> {x, y, z}, m_w {w} {}
    constexpr V4Base(V3Base<T> v3, T w) : V3Base<T> {v3}, m_w {w} {}
    template<typename Y> constexpr V4Base(V4Base<Y> v4) : V4Base(T(v4.x()), T(v4.y()), T(v4.z()), T(v4.w())) {}

    /* */

    T (&data())[4] { return *reinterpret_cast<T(*)[4]>(this); }
    const T (&data() const)[4] { return *reinterpret_cast<const T(*)[4]>(this); }

    T& w() { return m_w; }
    const T& w() const { return m_w; }

    T& a() { return w(); }
    const T& a() const { return w(); }

    V2Base<T>& zw() { return *(static_cast<V2Base<T>*>(this) + 1); }
    const V2Base<T>& zw() const { return *(static_cast<const V2Base<T>*>(this) + 1); }

    V3Base<T>& xyz() { return *(static_cast<V3Base<T>*>(this)); }
    const V3Base<T>& xyz() const { return *(static_cast<const V3Base<T>*>(this)); }

    T& operator[](int i) { ADT_ASSERT(i >= 0 && i < 4, "out of range i: {}", i); return *(reinterpret_cast<T*>(this) + i); }
    const T& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 4, "out of range i: {}", i); return *(reinterpret_cast<const T*>(this) + i); }
};

template<typename T>
inline V4Base<T> operator-(const V4Base<T>& a);

template<typename T>
inline V4Base<T> operator+(const V4Base<T>& a, const V4Base<T>& b);

template<typename T>
inline V4Base<T> operator+(const V4Base<T>& a, T b);

template<typename T>
inline V4Base<T> operator-(const V4Base<T>& a, const V4Base<T>& b);

template<typename T>
inline V4Base<T> operator-(const V4Base<T>& a, T b);

template<typename T>
inline V4Base<T> operator*(const V4Base<T>& a, const V4Base<T>& b);

template<typename T>
inline V4Base<T> operator*(const V4Base<T>& a, T b);

template<typename T>
struct M2Base
{
    V2Base<T> m_aData[2];

    /* */

    M2Base() = default;

    /* */

    T (&data())[2][2] { return *reinterpret_cast<T(*)[2][2]>(this); }
    const T (&data() const)[2][2] { return *reinterpret_cast<const T(*)[2][2]>(this); }
};

template<typename T>
struct M3Base
{
    V3Base<T> m_aData[3];

    /* */

    M3Base() = default;
    explicit M3Base(InitFlag);
    template<typename Y> M3Base(const M4Base<Y>& m4);

    /* */

    T (&data())[3][3] { return *reinterpret_cast<T(*)[3][3]>(this); }
    const T (&data() const)[3][3] { return *reinterpret_cast<const T(*)[3][3]>(this); }
};

template<typename T>
inline M3Base<T> M3Iden();

template<typename T>
inline M3Base<T> operator*(const M3Base<T>& a, const M3Base<T>& b);

template<typename T>
inline T M3Det(const M3Base<T>& s);

template<typename T>
struct M4Base
{
    V4Base<T> m_aData[4];

    /* */

    M4Base() = default;
    M4Base(T _0, T _1, T _2, T _3, T _4, T _5, T _6, T _7, T _8, T _9, T _10, T _11, T _12, T _13, T _14, T _15);
    M4Base(const T (&a)[16]) : m_aData{{a[0], a[1], a[2], a[3]}, {a[4], a[5], a[6], a[7]}, {a[8], a[9], a[10], a[11]}, {a[12], a[13], a[14], a[15]}} {}
    explicit M4Base(InitFlag);

    /* */

    T (&ptr())[16] { return *reinterpret_cast<T(*)[16]>(this); }
    const T (&ptr() const)[16] { return *reinterpret_cast<const T(*)[16]>(this); }

    T (&data())[4][4] { return *reinterpret_cast<T(*)[4][4]>(this); }
    const T (&data() const)[4][4] { return *reinterpret_cast<const T(*)[4][4]>(this); }

    V4Base<T>& operator[](int i) { ADT_ASSERT(i >= 0 && i < 4, "out of range i: {}", i); return m_aData[i]; }
    const V4Base<T>& operator[](int i) const { ADT_ASSERT(i >= 0 && i < 4, "out of range i: {}", i); return m_aData[i]; }
};

template<typename T = f32>
inline M4Base<T> M4Iden();

template<typename T>
inline M4Base<T> M4Tra(T x, T y, T z);

template<typename T>
inline M4Base<T> M4Tra(V3Base<T> t);

template<typename T>
inline M4Base<T> M4Rot(T theta, V3Base<T> axis);

template<typename T>
inline M4Base<T> M4Rot(T x, T y, T z);

template<typename T>
inline M4Base<T> M4RotX(T theta);

template<typename T>
inline M4Base<T> M4RotY(T theta);

template<typename T>
inline M4Base<T> M4RotZ(T theta);

template<typename T>
inline M4Base<T> M4Sca(T s);

template<typename T>
inline M4Base<T> M4Sca(V3Base<T> s);

template<typename T>
inline T M4Det(const M4Base<T>& s);

template<typename T>
inline M4Base<T> M4Adj(const M4Base<T>& s);

template<typename T>
inline M4Base<T> operator*(const M4Base<T>& a, const M4Base<T>& b);

template<typename T>
inline M4Base<T> operator*=(M4Base<T>& a, const M4Base<T>& b);

template<typename T>
inline V4Base<T> operator*(const M4Base<T>& a, const V4Base<T>& b);

using V2 = V2Base<f32>;
using IV2 = V2Base<i32>;

using V3 = V3Base<f32>;
using IV3 = V3Base<i32>;

using V4 = V4Base<f32>;
using IV4 = V4Base<i32>;

using M2 = M2Base<f32>;
using M3 = M3Base<f32>;
using M4 = M4Base<f32>;

struct Qt : V4Base<f32>
{
    using V4Base<f32>::V4Base;

    /* */

    Qt() = default;
    Qt(V4Base<f32> v4);

    /* */

    M4 matrix() const;
};

inline Qt QtIden();

inline Qt QtAxisAngle(const V3& axis, f32 th);

template<typename T>
inline T length(const V3Base<T>& v);

template<typename T>
inline V3Base<T> norm(const V3Base<T>& v);

template<typename T>
inline V3Base<T> norm(const V3Base<T>& v, T len);

inline Qt norm(const Qt& v);

template<typename T>
inline V3 cross(const V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline T dot(const V3Base<T>& a, const V3Base<T>& b);

template<typename T>
inline T dot(const V4Base<T>& a, const V4Base<T>& b);

template<typename T>
inline M4Base<T> transpose(const M4Base<T>& m);

constexpr inline auto lerp(const auto& a, const auto& b, const auto& t);
inline Qt slerp(const Qt& q1, const Qt& q2, f32 t);

inline M4 M4Pers(f32 fov, f32 asp, f32 n, f32 f);
inline M4 M4Ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
inline M4 transform(const V3& pos, const Qt& rot, const V3& scale);

} /* namespace adt::math */

namespace adt::print
{

template<typename T>
inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::V2Base<T>& x)
{
    return formatToContext(ctx, fmtArgs, x.data());
}

template<typename T>
inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::V3Base<T>& x)
{
    return formatToContext(ctx, fmtArgs, x.data());
}

template<typename T>
inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::V4Base<T>& x)
{
    return formatToContext(ctx, fmtArgs, x.data());
}

template<typename T>
inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::M3Base<T>& x)
{
    auto& e = x.data();

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

template<typename T>
inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const adt::math::M4Base<T>& x)
{
    auto& e = x.data();

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
