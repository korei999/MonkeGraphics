#pragma once

#include "StringDecl.hh"

#include "assert.hh"
#include "IAllocator.hh"
#include "utils.hh"
#include "hash.hh"
#include "Span.hh" /* IWYU pragma: keep */
#include "print.hh" /* IWYU pragma: keep */

#include <cstdlib>

namespace adt
{

[[nodiscard]] constexpr ssize
ntsSize(const char* nts)
{
    ssize i = 0;
    if (!nts) return 0;

#if defined __has_constexpr_builtin

    #if __has_constexpr_builtin(__builtin_strlen)
    i = __builtin_strlen(nts);
    #endif

#else

    while (nts[i] != '\0') ++i;

#endif

    return i;
}

template <ssize SIZE>
[[nodiscard]] constexpr ssize
charBuffStringSize(const char (&aCharBuff)[SIZE])
{
    ssize i = 0;
    if (SIZE == 0) return 0;

    while (i < SIZE && aCharBuff[i] != '\0') ++i;

    return i;
}

inline constexpr
StringView::StringView(const char* nts)
    : m_pData(const_cast<char*>(nts)), m_size(ntsSize(nts)) {}

inline constexpr
StringView::StringView(char* pStr, ssize len)
    : m_pData(pStr), m_size(len) {}

inline constexpr
StringView::StringView(Span<char> sp)
    : StringView(sp.data(), sp.size()) {}

template<ssize SIZE>
inline constexpr
StringView::StringView(const char (&aCharBuff)[SIZE])
    : m_pData(const_cast<char*>(aCharBuff)),
      m_size(charBuffStringSize(aCharBuff)) {}

#define ADT_RANGE_CHECK ADT_ASSERT(i >= 0 && i < m_size, "i: {}, m_size: {}", i, m_size);

inline constexpr char&
StringView::operator[](ssize i)
{
    ADT_RANGE_CHECK; return m_pData[i];
}

constexpr const char&
StringView::operator[](ssize i) const
{
    ADT_RANGE_CHECK; return m_pData[i];
}

#undef ADT_RANGE_CHECK

/* wchar_t iterator for mutlibyte strings */
struct StringGlyphIt
{
    const StringView m_s;

    /* */

    StringGlyphIt(const StringView s) : m_s(s) {};

    /* */

    struct It
    {
        const char* p {};
        ssize i = 0;
        ssize size = 0;
        wchar_t wc {};

        It(const char* pFirst, ssize _i, ssize _size)
            : p{pFirst}, i(_i), size(_size)
        {
            if (i != NPOS) operator++();
        }

        wchar_t& operator*() { return wc; }
        wchar_t* operator->() { return &wc; }

        It
        operator++()
        {
            if (!p || i < 0 || i >= size)
            {
GOTO_quit:
                i = NPOS;
                return *this;
            }

            int len = 0;

            len = mbtowc(&wc, &p[i], size - i);

            if (len == -1)
                goto GOTO_quit;
            else if (len == 0)
                len = 1;

            i += len;

            return *this;
        }

        friend bool operator==(const It& l, const It& r) { return l.i == r.i; }
        friend bool operator!=(const It& l, const It& r) { return l.i != r.i; }
    };

    It begin() { return {m_s.data(), 0, m_s.size()}; }
    It end() { return {{}, NPOS, {}}; }

    const It begin() const { return {m_s.data(), 0, m_s.size()}; }
    const It end() const { return {{}, NPOS, {}}; }
};

/* Separated by delimiter String iterator adapter */
struct StringWordIt
{
    const StringView m_sv {};
    const StringView m_svDelimiters {};

    /* */

    StringWordIt(const StringView sv, const StringView svDelimiters = " ") : m_sv(sv), m_svDelimiters(svDelimiters) {}

    struct It
    {
        StringView m_svCurrWord {};
        const StringView m_svStr;
        const StringView m_svSeps {};
        ssize m_i = 0;

        /* */

        It(const StringView sv, ssize pos, const StringView svSeps, bool)
            : m_svStr(sv), m_svSeps(svSeps),  m_i(pos)
        {
            if (pos != NPOS) operator++();
        }

        It(const StringView sv, ssize pos, const StringView svSeps)
            : m_svStr(sv), m_svSeps(svSeps),  m_i(pos) {}

        explicit It(ssize i) : m_i(i) {}

        /* */

        auto& operator*() { return m_svCurrWord; }
        auto* operator->() { return &m_svCurrWord; }

        It&
        operator++()
        {
            if (m_i >= m_svStr.size())
            {
                m_i = NPOS;
                return *this;
            }

            ssize start = m_i;
            ssize end = m_i;

            auto oneOf = [&](char c) -> bool
            {
                for (auto sep : m_svSeps)
                    if (c == sep)
                        return true;

                return false;
            };

            while (end < m_svStr.size() && !oneOf(m_svStr[end]))
                end++;

            m_svCurrWord = {const_cast<char*>(&m_svStr[start]), end - start};
            m_i = end + 1;

            if (m_svCurrWord.empty()) operator++();

            return *this;
        }

        friend bool operator==(const It& l, const It& r) { return l.m_i == r.m_i; }
        friend bool operator!=(const It& l, const It& r) { return l.m_i != r.m_i; }
    };

    It begin() { return {m_sv, 0, m_svDelimiters, true}; }
    It end() { return It(NPOS); }

    const It begin() const { return {m_sv, 0, m_svDelimiters, true}; }
    const It end() const { return It(NPOS); }
};

constexpr inline ssize
StringView::idx(const char* const p) const
{
    ssize i = p - m_pData;
    ADT_ASSERT(i >= 0 && i < size(), "out of range: idx: {}: size: {}", i, size());

    return i;
}

inline bool
StringView::beginsWith(const StringView r) const
{
    const auto& l = *this;

    if (l.size() < r.size())
        return false;

    for (ssize i = 0; i < r.size(); ++i)
        if (l[i] != r[i])
            return false;

    return true;
}

inline bool
StringView::endsWith(const StringView r) const
{
    const auto& l = *this;

    if (l.m_size < r.m_size)
        return false;

    for (ssize i = r.m_size - 1, j = l.m_size - 1; i >= 0; --i, --j)
        if (r[i] != l[j])
            return false;

    return true;
}

inline bool
operator==(const StringView& l, const StringView& r)
{
    if (l.data() == r.data())
        return true;

    if (l.size() != r.size())
        return false;

    return strncmp(l.data(), r.data(), l.size()) == 0; /* strncmp is as fast as handmade AVX2 function. */
}

inline bool
operator==(const StringView& l, const char* r)
{
    auto sr = StringView(r);
    return l == sr;
}

inline bool
operator!=(const StringView& l, const StringView& r)
{
    return !(l == r);
}

inline i64
operator-(const StringView& l, const StringView& r)
{
    if (l.m_size < r.m_size) return -1;
    else if (l.m_size > r.m_size) return 1;

    i64 sum = 0;
    for (ssize i = 0; i < l.m_size; i++)
        sum += (l[i] - r[i]);

    return sum;
}

inline ssize
StringView::lastOf(char c) const
{
    for (int i = m_size - 1; i >= 0; i--)
        if ((*this)[i] == c)
            return i;

    return NPOS;
}

inline void
StringView::trimEnd()
{
    auto isWhiteSpace = [&](int i) -> bool {
        char c = m_pData[i];
        if (c == '\n' || c == ' ' || c == '\r' || c == '\t' || c == '\0')
            return true;

        return false;
    };

    for (int i = m_size - 1; i >= 0; --i)
        if (isWhiteSpace(i))
        {
            m_pData[i] = 0;
            --m_size;
        }
        else break;
}

inline void
StringView::removeNLEnd()
{
    auto oneOf = [&](const char c) -> bool {
        constexpr StringView chars = "\r\n";
        for (const char ch : chars)
            if (c == ch) return true;
        return false;
    };

    while (m_size > 0 && oneOf(last()))
        m_pData[--m_size] = '\0';
}

inline bool
StringView::contains(const StringView r) const
{
    if (m_size < r.m_size || m_size == 0 || r.m_size == 0) return false;

    for (ssize i = 0; i < m_size - r.m_size + 1; ++i)
    {
        const StringView sSub {const_cast<char*>(&(*this)[i]), r.m_size};
        if (sSub == r)
            return true;
    }

    return false;
}

inline char&
StringView::first()
{
    return operator[](0);
}

inline const char&
StringView::first() const
{
    return operator[](0);
}

inline char&
StringView::last()
{
    return operator[](m_size - 1);
}

inline const char&
StringView::last() const
{
    return operator[](m_size - 1);
}

inline ssize
StringView::nGlyphs() const
{
    ssize n = 0;
    for (ssize i = 0; i < m_size; )
    {
        i+= mblen(&operator[](i), size() - i);
        ++n;
    }

    return n;
}

template<typename T>
ADT_NO_UB inline T
StringView::reinterpret(ssize at) const
{
    return *(T*)(&operator[](at));
}

inline
String::String(IAllocator* pAlloc, const char* pChars, ssize size)
{
    if (pChars == nullptr || size <= 0)
        return;

    char* pNewData = pAlloc->mallocV<char>(size + 1);
    memcpy(pNewData, pChars, size);
    pNewData[size] = '\0';

    m_pData = pNewData;
    m_size = size;
}

inline
String::String(IAllocator* pAlloc, const char* nts)
    : String(pAlloc, nts, ntsSize(nts)) {}

inline
String::String(IAllocator* pAlloc, Span<char> spChars)
    : String(pAlloc, spChars.data(), spChars.size()) {}

inline
String::String(IAllocator* pAlloc, const StringView sv)
    : String(pAlloc, sv.data(), sv.size()) {}

inline void
String::destroy(IAllocator* pAlloc)
{
    pAlloc->free(m_pData);
    *this = {};
}

inline void
String::replaceWith(IAllocator* pAlloc, StringView svWith)
{
    if (svWith.empty())
    {
        destroy(pAlloc);
        return;
    }

    if (size() < svWith.size() + 1)
        m_pData = pAlloc->reallocV<char>(data(), 0, svWith.size() + 1);

    strncpy(data(), svWith.data(), svWith.size());
    m_size = svWith.size();
    data()[size()] = '\0';
}

template<int SIZE>
inline
StringFixed<SIZE>::StringFixed(const StringView svName)
{
    /* memcpy doesn't like nullptrs */
    if (!svName.data() || svName.size() <= 0) return;

    memcpy(m_aBuff,
        svName.data(),
        utils::min(svName.size(), static_cast<ssize>(sizeof(m_aBuff)))
    );
}

template<int SIZE>
template<int SIZE_B>
inline
StringFixed<SIZE>::StringFixed(const StringFixed<SIZE_B> other)
{
    memcpy(m_aBuff, other.m_aBuff, utils::min(SIZE, SIZE_B));
}

template<int SIZE>
inline ssize
StringFixed<SIZE>::size() const
{
    return strnlen(m_aBuff, SIZE);
}

template<int SIZE>
inline bool
StringFixed<SIZE>::operator==(const StringFixed<SIZE>& other) const
{
    return memcmp(m_aBuff, other.m_aBuff, SIZE) == 0;
}

template<int SIZE>
inline bool
StringFixed<SIZE>::operator==(const StringView sv) const
{
    return StringView(m_aBuff) == sv;
}

template<int SIZE_L, int SIZE_R>
inline bool
operator==(const StringFixed<SIZE_L>& l, const StringFixed<SIZE_R>& r)
{
    return memcmp(l.m_aBuff, r.m_aBuff, utils::min(SIZE_L, SIZE_R)) == 0;
}

inline String
StringCat(IAllocator* p, const StringView& l, const StringView& r)
{
    ssize len = l.size() + r.size();
    char* ret = (char*)p->zalloc(len + 1, sizeof(char));

    ssize pos = 0;
    for (ssize i = 0; i < l.size(); ++i, ++pos)
        ret[pos] = l[i];
    for (ssize i = 0; i < r.size(); ++i, ++pos)
        ret[pos] = r[i];

    ret[len] = '\0';

    String sNew;
    sNew.m_pData = ret;
    sNew.m_size = len;
    return sNew;
}

template<>
inline usize
hash::func(const StringView& str)
{
    return hash::func(str.m_pData, str.size());
}

} /* namespace adt */
