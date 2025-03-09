#pragma once

#include "String.hh"

#include <cstdio>

namespace adt
{

#if __has_include(<dirent.h>)

    #define ADT_USE_DIRENT
    #include <dirent.h>

#elif __has_include(<windows.h>)

    #define ADT_USE_WIN32DIR
    #warning "TODO"

#else

    #error "unsupported platform"

#endif

struct Directory
{
    DIR* m_pDir {};

    /* */

    Directory() = default;
    Directory(const char* ntsPath);

    /* */

    explicit operator bool() const { return m_pDir != nullptr; }

    /* */

    bool close();

    /* */

    struct It
    {
        Directory* p {};
        dirent* pEntry {};
        int i {}; /* NPOS flag */

        /* */

        It(const Directory* pSelf, int _i)
            : p(const_cast<Directory*>(pSelf)), i(_i)
        {
            if (i != NPOS)
            {
                /* skip '.' and '..' */
                while ((pEntry = readdir(p->m_pDir)))
                {
                    if (strncmp(pEntry->d_name, ".", sizeof(pEntry->d_name)) == 0 ||
                        strncmp(pEntry->d_name, "..", sizeof(pEntry->d_name)) == 0)
                        continue;
                    else break;
                }
            }
        }

        /* */

        [[nodiscard]] StringView
        operator*()
        {
            if (!pEntry) return {};

            usize n = strnlen(pEntry->d_name, sizeof(pEntry->d_name));
            return {pEntry->d_name, static_cast<ssize>(n)};
        }

        It
        operator++()
        {
            pEntry = readdir(p->m_pDir);
            if (!pEntry) i = NPOS;

            return *this;
        }

        friend bool operator==(const It& l, const It& r) { return l.i == r.i; }
        friend bool operator!=(const It& l, const It& r) { return l.i != r.i; }
    };

    It begin() { return {this, 0}; }
    It end() { return {this, NPOS}; }

    It begin() const { return {this, 0}; }
    It end() const { return {this, NPOS}; }
};

inline
Directory::Directory(const char* ntsPath)
{
    m_pDir = opendir(ntsPath);
    if (!m_pDir)
    {
#ifndef NDEBUG
        fprintf(stderr, "failed to open '%s' directory\n", ntsPath);
#endif
        return;
    }


}

inline bool
Directory::close()
{
     int err = closedir(m_pDir);

#ifndef NDEBUG
     if (err != 0) fprintf(stderr, "closedir(): error '%d'\n", err);
#endif

     return err == 0;
}

} /* namespace adt */
