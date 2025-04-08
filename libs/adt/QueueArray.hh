#pragma once

#include "types.hh"

#include <utility>

namespace adt
{

template<typename T, ssize CAP>
struct QueueArray
{
    static_assert(isPowerOf2(CAP), "CAP must be a power of two");

    /* */

    T m_aData[CAP] {};
    ssize m_headI {};
    ssize m_tailI {};

    /* */

    bool empty() const;

    ssize pushBack(const T& x); /* Index of inserted element, -1 on failure. */

    ssize pushFront(const T& x);

    template<typename ...ARGS>
    ssize emplaceBack(ARGS&&... args);

    template<typename ...ARGS>
    ssize emplaceFront(ARGS&&... args);

    T* popFront();

    T* popBack();

protected:
    enum PUSH_WAY : u8 { HEAD, TAIL };

    /* */

    template<PUSH_WAY E_WAY>
    ssize pushIdx();
};

template<typename T, ssize CAP>
inline bool
QueueArray<T, CAP>::empty() const
{
    return m_headI == m_tailI;
}

template<typename T, ssize CAP>
inline ssize
QueueArray<T, CAP>::pushBack(const T& x)
{
    ssize i = pushIdx<PUSH_WAY::TAIL>();
    if (i >= 0) new(m_aData + i) T(x);

    return i;
}

template<typename T, ssize CAP>
inline ssize
QueueArray<T, CAP>::pushFront(const T& x)
{
    ssize i = pushIdx<PUSH_WAY::HEAD>();
    if (i >= 0) new(m_aData + i) T(x);

    return i;
}

template<typename T, ssize CAP>
template<typename ...ARGS>
inline ssize
QueueArray<T, CAP>::emplaceBack(ARGS&&... args)
{
    ssize i = pushIdx<PUSH_WAY::TAIL>();
    if (i >= 0) new(m_aData + i) T(std::forward<ARGS>(args)...);

    return i;
}

template<typename T, ssize CAP>
template<typename ...ARGS>
inline ssize
QueueArray<T, CAP>::emplaceFront(ARGS&&... args)
{
    ssize i = pushIdx<PUSH_WAY::HEAD>();
    if (i >= 0) new(m_aData + i) T(std::forward<ARGS>(args)...);

    return i;
}

template<typename T, ssize CAP>
template<QueueArray<T, CAP>::PUSH_WAY E_WAY>
inline ssize
QueueArray<T, CAP>::pushIdx()
{
    if constexpr (E_WAY == PUSH_WAY::TAIL)
    {
        ssize nextTailI = (m_tailI + 1) & (CAP - 1); /* power of 2 wrapping */
        if (nextTailI == m_headI) return -1; /* full case */

        ssize prevTailI = m_tailI;
        m_tailI = nextTailI;
        return prevTailI;
    }
    else
    {
        ssize nextHeadI = (m_headI - 1) & (CAP - 1);
        if (nextHeadI == m_tailI) return -1;

        ssize prevHeadI = m_headI;
        m_headI = nextHeadI;
        return prevHeadI;
    }
}

template<typename T, ssize CAP>
inline T*
QueueArray<T, CAP>::popFront()
{
    if (empty()) return nullptr;

    ssize nextHeadI = (m_headI + 1) & (CAP - 1);
    ssize prevHeadI = m_headI;
    m_headI = nextHeadI;

    return &m_aData[prevHeadI];
}

template<typename T, ssize CAP>
inline T*
QueueArray<T, CAP>::popBack()
{
    if (empty()) return nullptr;

    ssize nextTailI = (m_tailI - 1) & (CAP - 1);
    ssize prevTailI = m_tailI;
    m_tailI = nextTailI;

    return &m_aData[prevTailI];
}

} /* namespace adt */
