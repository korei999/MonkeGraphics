#pragma once

#include "adt/types.hh"

namespace adt
{

/* Span with byteStride. */
template<typename T>
struct View
{
    T* m_pData {};
    ssize m_size {}; /* element count (not byte size) */
    ssize m_byteStride {};

    /* */

    View() = default;
    View(const T* pData, ssize size, ssize stride);

    /* */

    T& operator[](ssize i) { return at(i); }
    const T& operator[](ssize i) const { return at(i); }

    ssize getSize() const { return m_size; }
    ssize getStride() const { return m_byteStride; }
    ssize idx(const T* pElement);
    u8* u8Data() { return (u8*)(m_pData); }
    const u8* u8Data() const { return (const u8*)(m_pData); }

private:
    T& at(ssize i) const;

    /* */
public:

    struct It
    {
        View* pView;
        ssize i {};

        /* */

        It(const View* _self, ssize _i) : pView(const_cast<View*>(_self)), i(_i) {}

        /* */

        T& operator*() noexcept { return pView->operator[](i); }
        T* operator->() noexcept { return &pView->operator[](i); }

        It operator++() noexcept { ++i; return *this; }

        It operator--() noexcept { --i; return *this; }

        friend constexpr bool operator==(const It& l, const It& r) noexcept { return l.i == r.i; }
        friend constexpr bool operator!=(const It& l, const It& r) noexcept { return l.i != r.i; }
    };

    It begin()  noexcept { return {this, 0}; }
    It end()    noexcept { return {this, m_size}; }
    It rbegin() noexcept { return {this, m_size - 1}; }
    It rend()   noexcept { return {this, NPOS}; }

    const It begin()  const noexcept { return {this, 0}; }
    const It end()    const noexcept { return {this, m_size}; }
    const It rbegin() const noexcept { return {this, m_size - 1}; }
    const It rend()   const noexcept { return {this, NPOS}; }
};


template<typename T>
View<T>::View(const T* pData, ssize size, ssize stride)
    : m_pData(const_cast<T*>(pData)), m_size(size), m_byteStride(stride)
{
    ADT_ASSERT(m_byteStride > 0, " ");
}

template<typename T>
inline T&
View<T>::at(ssize i) const
{
    ADT_ASSERT(i >= 0 && i < m_size, "i: %lld, size: %lld, stride: %lld", i, m_size, m_byteStride);

    auto* pU8 = u8Data();
    T* pRet = (T*)(pU8 + (i*m_byteStride));
    return *pRet;
}

template<typename T>
inline ssize
View<T>::idx(const T* pElement)
{
    auto* p = reinterpret_cast<const u8*>(pElement);
    pdiff absIdx = p - u8Data();
    return absIdx / m_byteStride;
}

} /* namespace adt */
