#pragma once

#include "IAllocator.hh"
#include "Span.hh"

#include <cstring>

namespace adt
{

/* Each nextMem() invalidates previous nextMem() span. */
struct ScratchBuffer
{
    Span<u8> m_sp {};
    ssize m_pos {};

    /* */

    ScratchBuffer() = default;

    template<typename T>
    ScratchBuffer(Span<T> sp) noexcept
        : m_sp((u8*)sp.data(), sp.size() * sizeof(T)) {}

    template<typename T, usize SIZE>
    ScratchBuffer(T (&aBuff)[SIZE]) noexcept
        : m_sp((u8*)aBuff, SIZE * sizeof(T)) {}

    /* */

    template<typename T>
    [[nodiscard]] Span<T> nextMem(ssize mCount) noexcept;

    /* nextMem() + memset() to zero */
    template<typename T>
    [[nodiscard]] Span<T> nextMemZero(ssize mCount) noexcept;

    template<typename T>
    [[nodiscard]] Span<T> allMem() noexcept;

    void zeroOut() noexcept;

    ssize cap() noexcept { return m_sp.size(); }
};


template<typename T>
inline Span<T>
ScratchBuffer::nextMem(ssize mCount) noexcept
{
    const ssize realSize = align8(mCount * sizeof(T));

    if (realSize >= m_sp.size())
    {
        fprintf(stderr, "ScratchBuffer::nextMem(): allocating more than capacity (%lld < %lld), returing full buffer\n", m_sp.size(), realSize);
        return {(T*)m_sp.data(), ssize(cap() / sizeof(T))};
    }
    else if (realSize + m_pos > m_sp.size())
    {
        m_pos = 0;
    }

    void* pMem = &m_sp[m_pos];
    m_pos += realSize;

    return {(T*)pMem, mCount};
}

template<typename T>
inline Span<T>
ScratchBuffer::nextMemZero(ssize mCount) noexcept
{
    auto sp = nextMem<T>(mCount);
    memset(sp.data(), 0, sp.size() * sizeof(T));
    return sp;
}

template<typename T>
Span<T>
ScratchBuffer::allMem() noexcept
{
    return {reinterpret_cast<T*>(m_sp.data()), m_sp.size() / sizeof(T)};
}

inline void
ScratchBuffer::zeroOut() noexcept
{
    m_pos = 0;
    memset(m_sp.data(), 0, m_sp.size());
}

} /* namespace adt */
