#pragma once

#include "mathDecl.hh"

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

template<typename T>
inline constexpr
V2Base<T>::V2Base(T x, T y) : m_aData {x, y} {}

template<typename T>
inline constexpr
V2Base<T>::V2Base(const T (&a)[2]) : m_aData {a[0], a[1]} {}

template<typename T>
template<typename Y>
inline constexpr
V2Base<T>::V2Base(V2Base<Y> v2) : m_aData {T(v2.x()), T(v2.y())} {}

template<typename T>
template<typename Y>
inline constexpr
V2Base<T>::V2Base(V3Base<Y> v3) : m_aData {v3.x(), v3.y()} {}

template<typename T>
inline V2Base<T>
V2Base<T>::operator-() const
{
    return {
        -x(), -y()
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator+(const V2Base<T> b) const
{
    return {
        x() + b.x(),
        y() + b.y()
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator+(T b) const
{
    return {
        x() + b,
        y() + b
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator-(const V2Base<T> b) const
{
    return {
        x() - b.x(),
        y() - b.y()
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator-(T b) const
{
    return {
        x() - b,
        y() - b
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator*(const V2Base<T> b) const
{
    return {
        x() * b.x(),
        y() * b.y()
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator*(T b) const
{
    return {
        x() * b,
        y() * b
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator/(const V2Base<T> b) const
{
    return {
        x() / b.x(),
        y() / b.y()
    };
}

template<typename T>
inline V2Base<T>
V2Base<T>::operator/(T b) const
{
    return {
        x() / b,
        y() / b
    };
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator+=(const V2Base b)
{
    x() += b.x();
    y() += b.y();

    return *this;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator+=(T b)
{
    x() += b;
    y() += b;

    return *this;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator-=(const V2Base b)
{
    x() -= b.x();
    y() -= b.y();

    return *this;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator-=(T b)
{
    x() -= b;
    y() -= b;

    return *this;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator*=(const V2Base b)
{
    x() *= b.x();
    y() *= b.y();

    return *this;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator*=(T b)
{
    x() *= b;
    y() *= b;

    return *this;
}

template<typename T>
inline V2Base<T>&
V2Base<T>::operator/=(const V2Base b)
{
    x() /= b.x();
    y() /= b.y();

    return *this;
}

/* V2 end */

/* V3 */

template<typename T>
inline constexpr V3Base<T>::V3Base(const T (&a)[3]) : V2Base<T> {a[0], a[1]}, m_z {a[2]} {}

template<typename T>
inline constexpr V3Base<T>::V3Base(T x, T y, T z) : V2Base<T> {x, y}, m_z {z} {}

template<typename T>
inline V3Base<T>
operator-(const V3Base<T>& a)
{
    return {
        -a.x(), -a.y(), -a.z()
    };
}

template<typename T>
inline V3Base<T>
operator+(const V3Base<T>& a, const V3Base<T>& b)
{
    return {
        a.x() + b.x(),
        a.y() + b.y(),
        a.z() + b.z(),
    };
}

template<typename T>
inline V3Base<T>&
operator+=(V3Base<T>& a, const V3Base<T>& b)
{
    a.x() += b.x();
    a.y() += b.y();
    a.z() += b.z();

    return a;
}

template<typename T>
inline V3Base<T>
operator+(const V3Base<T>& a, T b)
{
    return {
        a.x() + b,
        a.y() + b,
        a.z() + b,
    };
}

template<typename T>
inline V3Base<T>&
operator+=(V3Base<T>& a, T b)
{
    a.x() += b;
    a.y() += b;
    a.z() += b;

    return a;
}

template<typename T>
inline V3Base<T>
operator-(const V3Base<T>& a, const V3Base<T>& b)
{
    return {
        a.x() - b.x(),
        a.y() - b.y(),
        a.z() - b.z(),
    };
}

template<typename T>
inline V3Base<T>&
operator-=(V3Base<T>& a, const V3Base<T>& b)
{
    a.x() -= b.x();
    a.y() -= b.y();
    a.z() -= b.z();

    return a;
}

template<typename T>
inline V3Base<T>&
operator-=(V3Base<T>& a, T b)
{
    a.x() -= b;
    a.y() -= b;
    a.z() -= b;

    return a;
}

template<typename T>
inline V3Base<T>
operator-(const V3Base<T>& a, T b)
{
    return {
        a.x() - b,
        a.y() - b,
        a.z() - b,
    };
}

template<typename T>
inline V3Base<T>
operator*(const V3Base<T>& a, const V3Base<T>& b)
{
    return {
        a.x() * b.x(),
        a.y() * b.y(),
        a.z() * b.z(),
    };
}

template<typename T>
inline V3Base<T>&
operator*=(V3Base<T>& a, const V3Base<T>& b)
{
    a.x() *= b.x();
    a.y() *= b.y();
    a.z() *= b.z();

    return a;
}

template<typename T>
inline V3Base<T>
operator*(const V3Base<T>& a, T b)
{
    return {
        a.x() * b,
        a.y() * b,
        a.z() * b,
    };
}

template<typename T>
inline V3Base<T> operator*(T a, const V3Base<T>& b)
{
    return b * a;
}

template<typename T>
inline V3Base<T>&
operator*=(V3Base<T>& a, T b)
{
    a.x() *= b;
    a.y() *= b;
    a.z() *= b;

    return a;
}

template<typename T>
inline V3Base<T>
operator/(const V3Base<T>& a, const V3Base<T>& b)
{
    return {
        a.x() / b.x(),
        a.y() / b.y(),
        a.z() / b.z(),
    };
}

template<typename T>
inline V3Base<T>&
operator/=(V3Base<T>& a, const V3Base<T>& b)
{
    a.x() /= b.x();
    a.y() /= b.y();
    a.z() /= b.z();

    return a;
}

template<typename T>
inline V3Base<T>
operator/(const V3Base<T>& a, T b)
{
    return {
        a.x() / b,
        a.y() / b,
        a.z() / b,
    };
}

template<typename T>
inline V3Base<T>& operator/=(V3Base<T>& a, T b)
{
    a.x() /= b;
    a.y() /= b;
    a.z() /= b;

    return a;
}

/* V3 end */

/* V4 */

template<typename T>
inline V4Base<T>
operator-(const V4Base<T>& a)
{
    return {
        -a.x(), -a.y(), -a.z(), -a.w()
    };
}

template<typename T>
inline V4Base<T>
operator+(const V4Base<T>& a, const V4Base<T>& b)
{
    return {
        a.x() + b.x(),
        a.y() + b.y(),
        a.z() + b.z(),
        a.w() + b.w()
    };
}

template<typename T>
inline V4Base<T>
operator+(const V4Base<T>& a, T b)
{
    return {
        a.x() + b,
        a.y() + b,
        a.z() + b,
        a.w() + b
    };
}

template<typename T>
inline V4Base<T>
operator-(const V4Base<T>& a, const V4Base<T>& b)
{
    return {
        a.x() - b.x(),
        a.y() - b.y(),
        a.z() - b.z(),
        a.w() - b.w()
    };
}

template<typename T>
inline V4Base<T>
operator-(const V4Base<T>& a, T b)
{
    return {
        a.x() - b,
        a.y() - b,
        a.z() - b,
        a.w() - b
    };
}

template<typename T>
inline V4Base<T> operator*(const V4Base<T>& a, const V4Base<T>& b)
{
    return {
        a.x() * b.x(),
        a.y() * b.y(),
        a.z() * b.z(),
        a.w() * b.w()
    };
}

template<typename T>
inline V4Base<T>
operator*(const V4Base<T>& a, T b)
{
    return {
        a.x() * b,
        a.y() * b,
        a.z() * b,
        a.w() * b
    };
}

/* V4 end */

/* M3 */

template<typename T>
inline M3Base<T>::M3Base(InitFlag)
    : m_aData {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    }
{
}

template<typename T>
template<typename Y>
inline M3Base<T>::M3Base(const M4Base<Y>& m4)
    : m_aData {{m4.m_aData[0]}, {m4.m_aData[1]}, {m4.m_aData[2]}}
{
}

template<typename T>
inline M3Base<T> M3Iden()
{
    return {{
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    }};
}

template<typename T>
inline M3Base<T>
operator*(const M3Base<T>& a, const M3Base<T>& b)
{
    M3Base<T> r {};

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                r[i][j] += a[i][k] * b[k][j];

    return r;
}

template<typename T>
inline T
M3Det(const M3Base<T>& a)
{
    return (
        a[0][0] * (a[1][1] * a[2][2] - a[2][1] * a[1][2]) -
        a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
        a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0])
    );
}

/* M3 end */

/* M4 */

template<typename T>
inline M4Base<T>::M4Base(T _0, T _1, T _2, T _3, T _4, T _5, T _6, T _7, T _8, T _9, T _10, T _11, T _12, T _13, T _14, T _15)
    : m_aData{{_0, _1, _2, _3}, {_4, _5, _6, _7}, {_8, _9, _10, _11}, {_12, _13, _14, _15}}
{
}

template<typename T>
inline M4Base<T>::M4Base(InitFlag)
    : m_aData {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }
{
}

template<typename T>
inline M4Base<T>
M4Iden()
{
    return {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};
}

template<typename T>
inline M4Base<T>
M4Sca(T s)
{
    return {
        s, 0, 0, 0,
        0, s, 0, 0,
        0, 0, s, 0,
        0, 0, 0, s
    };
}

template<typename T>
inline M4Base<T>
M4Sca(V3Base<T> s)
{
    return {
        s.x(), 0,     0,     0,
        0,     s.y(), 0,     0,
        0,     0,     s.z(), 0,
        0,     0,     0,     1
    };
}

template<typename T>
inline T
M4Det(const M4Base<T>& a)
{
    return (
        a[0][3] * a[1][2] * a[2][1] * a[3][0] - a[0][2] * a[1][3] * a[2][1] * a[3][0] -
        a[0][3] * a[1][1] * a[2][2] * a[3][0] + a[0][1] * a[1][3] * a[2][2] * a[3][0] +
        a[0][2] * a[1][1] * a[2][3] * a[3][0] - a[0][1] * a[1][2] * a[2][3] * a[3][0] -
        a[0][3] * a[1][2] * a[2][0] * a[3][1] + a[0][2] * a[1][3] * a[2][0] * a[3][1] +
        a[0][3] * a[1][0] * a[2][2] * a[3][1] - a[0][0] * a[1][3] * a[2][2] * a[3][1] -
        a[0][2] * a[1][0] * a[2][3] * a[3][1] + a[0][0] * a[1][2] * a[2][3] * a[3][1] +
        a[0][3] * a[1][1] * a[2][0] * a[3][2] - a[0][1] * a[1][3] * a[2][0] * a[3][2] -
        a[0][3] * a[1][0] * a[2][1] * a[3][2] + a[0][0] * a[1][3] * a[2][1] * a[3][2] +
        a[0][1] * a[1][0] * a[2][3] * a[3][2] - a[0][0] * a[1][1] * a[2][3] * a[3][2] -
        a[0][2] * a[1][1] * a[2][0] * a[3][3] + a[0][1] * a[1][2] * a[2][0] * a[3][3] +
        a[0][2] * a[1][0] * a[2][1] * a[3][3] - a[0][0] * a[1][2] * a[2][1] * a[3][3] -
        a[0][1] * a[1][0] * a[2][2] * a[3][3] + a[0][0] * a[1][1] * a[2][2] * a[3][3]
    );
}

template<typename T>
inline M4Base<T>
M4Adj(const M4Base<T>& s)
{
    return transpose(cofactors(s));
}

template<typename T>
inline M4Base<T>
M4Tra(T x, T y, T z)
{
    return {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, z, y, 1
    };
}

template<typename T>
inline M4Base<T> M4Tra(V3Base<T> t)
{
    return {
        1,     0,     0,     0,
        0,     1,     0,     0,
        0,     0,     1,     0,
        t.x(), t.y(), t.z(), 1
    };
}

template<typename T>
inline M4Base<T>
M4Rot(T theta, V3Base<T> axis)
{
    const f32 c = std::cos(theta);
    const f32 s = std::sin(theta);

    const f32 x = axis.x;
    const f32 y = axis.y;
    const f32 z = axis.z;

    return {
        ((1 - c)*sq(x)) + c, ((1 - c)*x*y) - s*z, ((1 - c)*x*z) + s*y, 0,
        ((1 - c)*x*y) + s*z, ((1 - c)*sq(y)) + c, ((1 - c)*y*z) - s*x, 0,
        ((1 - c)*x*z) - s*y, ((1 - c)*y*z) + s*x, ((1 - c)*sq(z)) + c, 0,
        0,                   0,                   0,                   1
    };
}

template<typename T>
inline M4Base<T>
M4Rot(T x, T y, T z)
{
    return M4RotZ(z) * M4RotY(y) * M4RotX(x);
}

template<typename T>
inline M4Base<T>
M4RotX(T th)
{
    return {
        1,  0,            0,            0,
        0,  std::cos(th), std::sin(th), 0,
        0, -std::sin(th), std::cos(th), 0,
        0,  0,            0,            1
    };
}

template<typename T>
inline M4Base<T>
M4RotY(T th)
{
    return {
        std::cos(th), 0,  std::sin(th),  0,
        0,            1,  0,             0,
       -std::sin(th), 0,  std::cos(th),  0,
        0,            0,  0,             1
    };
}

template<typename T>
inline M4Base<T>
M4RotZ(T th)
{
    return {
        std::cos(th),  std::sin(th), 0, 0,
       -std::sin(th),  std::cos(th), 0, 0,
        0,             0,            1, 0,
        0,             0,            0, 1
    };
}

template<typename T>
inline M4Base<T>
operator*(const M4Base<T>& a, const M4Base<T>& b)
{
    M4Base<T> r {};

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                r[i][j] += a[i][k] * b[k][j];

    return r;
}

template<typename T>
inline M4Base<T> operator*=(M4Base<T>& a, const M4Base<T>& b)
{
    return a * b;
}

template<typename T>
inline V4Base<T>
operator*(const M4Base<T>& a, const V4Base<T>& b)
{
    V4Base<T> r {};

    return r;
}

/* M4 end */

/* Qt */

inline Qt::Qt(V4Base<f32> v4)
    : V4Base<f32>(v4)
{
}

inline M4
Qt::matrix() const
{
    return {
        1 - 2*y()*y() - 2*z()*z(), 2*x()*y() + 2*w()*z(),     2*x()*z() - 2*w()*y(),     0,
        2*x()*y() - 2*w()*z(),     1 - 2*x()*x() - 2*z()*z(), 2*y()*z() + 2*w()*x(),     0,
        2*x()*z() + 2*w()*y(),     2*y()*z() - 2*w()*x(),     1 - 2*x()*x() - 2*y()*y(), 0,
        0,                         0,                         0,                         1
    };
}

inline Qt
QtIden()
{
    return {0, 0, 0, 1};
}

inline Qt
QtAxisAngle(const V3& axis, f32 th)
{
    const f32 sinTh = std::sin(th / 2.0f);

    return {
        axis.x() * sinTh,
        axis.y() * sinTh,
        axis.z() * sinTh,
        std::cos(th / 2.0f)
    };
}

/* Qt end */

template<typename T>
inline T
length(const V3Base<T>& v)
{
    return std::sqrt(sq(v.x()) + sq(v.y()) + sq(v.z()));
}

template<typename T>
inline V3Base<T>
norm(const V3Base<T>& v)
{
    return norm(v, length(v));
}

template<typename T>
inline V3Base<T>
norm(const V3Base<T>& v, T len)
{
    return {v.x() / len, v.y() / len, v.z() / len};
}

inline Qt
norm(const Qt& a)
{
    f32 mag = std::sqrt(a.w() * a.w() + a.x() * a.x() + a.y() * a.y() + a.z() * a.z());
    return {a.x() / mag, a.y() / mag, a.z() / mag, a.w() / mag};
}

constexpr inline auto
lerp(const auto& a, const auto& b, const auto& t)
{
    return (decltype(t)(1.0) - t) * a + t * b;
}

inline Qt
slerp(const Qt& q1, const Qt& q2, f32 t)
{
    f32 d = dot(q1, q2);

    Qt q2b = q2;
    if (d < 0.0f)
    {
        q2b = -q2b;
        d = -d;
    }

    if (d > 0.9995f)
    {
        Qt res;
        res.x() = q1.x() + t * (q2b.x() - q1.x());
        res.y() = q1.y() + t * (q2b.y() - q1.y());
        res.z() = q1.z() + t * (q2b.z() - q1.z());
        res.w() = q1.w() + t * (q2b.w() - q1.w());
        return norm(res);
    }

    f32 theta0 = std::acos(d);
    f32 theta = theta0 * t;

    f32 sinTheta0 = std::sin(theta0);
    f32 sinTheta = std::sin(theta);

    f32 s1 = std::cos(theta) - d * (sinTheta / sinTheta0);
    f32 s2 = sinTheta / sinTheta0;

    Qt res;
    res.x() = (s1 * q1.x()) + (s2 * q2b.x());
    res.y() = (s1 * q1.y()) + (s2 * q2b.y());
    res.z() = (s1 * q1.z()) + (s2 * q2b.z());
    res.w() = (s1 * q1.w()) + (s2 * q2b.w());
    return res;
}

template<typename T>
inline V3
cross(const V3Base<T>& a, const V3Base<T>& b)
{
    return {
        (a.y() * b.z()) - (b.y() * a.z()),
        (a.z() * b.x()) - (b.z() * a.x()),
        (a.x() * b.y()) - (b.x() * a.y())
    };
}

template<typename T>
inline T
dot(const V3Base<T>& a, const V3Base<T>& b)
{
    return (a.x() * b.x()) + (a.y() * b.y()) + (a.z() * b.z());
}

template<typename T>
inline T
dot(const V4Base<T>& a, const V4Base<T>& b)
{
    return (a.x() * b.x()) + (a.y() * b.y()) + (a.z() * b.z()) + (a.w() * b.w());
}

template<typename T>
inline M4Base<T>
transpose(const M4Base<T>& m)
{
    return {
        m[0][0], m[1][0], m[2][0], m[3][0],
        m[0][1], m[1][1], m[2][1], m[3][1],
        m[0][2], m[1][2], m[2][2], m[3][2],
        m[0][3], m[1][3], m[2][3], m[3][3],
    };
}

inline M4
M4Pers(f32 fov, f32 asp, f32 n, f32 f)
{
    M4 res {};

    res[0].x() = 1.0f / (asp * std::tan(fov * 0.5f));
    res[1].y() = 1.0f / (std::tan(fov * 0.5f));
    res[2].z() = -f / (n - f);
    res[3].z() = n * f / (n - f);
    res[2].w() = 1.0f;

    return res;
}

inline M4
M4Ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    return {
        2/(r-l),       0,            0,           0,
        0,             2/(t-b),      0,           0,
        0,             0,           -2/(f-n),     0,
        -(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1
    };
}

inline M4
transform(const V3& pos, const Qt& rot, const V3& scale)
{
    return M4Tra(pos) * rot.matrix() * M4Sca(scale);
}

} /* namespace adt::math */
