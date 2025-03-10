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

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN 1
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>

#else

    #error "unsupported platform"

#endif

#ifdef ADT_USE_DIRENT

struct Directory
{
    DIR* m_pDir {};
    dirent* m_pEntry {};

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
                    if (strncmp(p->m_pEntry->d_name, ".", sizeof(p->m_pEntry->d_name)) == 0 ||
                        strncmp(p->m_pEntry->d_name, "..", sizeof(p->m_pEntry->d_name)) == 0)
                        continue;
                    else break;
                }
            }
        }

        It(int _i) : i(_i) {}

        /* */

        [[nodiscard]] StringView
        operator*()
        {
            if (!p || (p && !p->m_pEntry)) return {};

            usize n = strnlen(p->m_pEntry->d_name, sizeof(p->m_pEntry->d_name));
            return {p->m_pEntry->d_name, static_cast<ssize>(n)};
        }

        It
        operator++()
        {
            p->m_pEntry = readdir(p->m_pDir);
            if (!p->m_pEntry) i = NPOS;

            return {i};
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

#endif /* ADT_USE_DIRENT */

#ifdef ADT_USE_WIN32DIR

struct Directory
{
    char m_aBuff[sizeof(WIN32_FIND_DATA::cFileName) + 3] {};
    WIN32_FIND_DATA m_fileData {};
    HANDLE m_hFind {};

    /* */

    Directory() = default;
    Directory(const char* ntsPath);

    /* */

    explicit operator bool() const;

    /* */

    bool close();

    /* */

    struct It
    {
        Directory* p {};
        bool bStatus {};

        /* */

        It(const Directory* self, bool _bStatus)
            : p(const_cast<Directory*>(self)), bStatus(_bStatus)
        {
            if (!bStatus) return;

            p->m_hFind = FindFirstFile(p->m_aBuff, &p->m_fileData);

#ifndef NDEBUG
            if (p->m_hFind == INVALID_HANDLE_VALUE)
                fprintf(stderr, "failed to open '%s'\n", p->m_aBuff);
#endif

            do
            {
                if (strcmp(p->m_fileData.cFileName, ".") == 0 || 
                    strcmp(p->m_fileData.cFileName, "..") == 0)
                    continue;
                else break;
            }
            while (FindNextFile(p->m_hFind, &p->m_fileData) != 0);
        }

        It(bool _bStatus) : bStatus(_bStatus) {}

        /* */

        [[nodiscard]] StringView
        operator*()
        {
            if (!p) return {};

            return StringView(p->m_fileData.cFileName,
                strnlen(p->m_fileData.cFileName, sizeof(p->m_fileData.cFileName))
            );
        }

        It
        operator++()
        {
            if (FindNextFile(p->m_hFind, &p->m_fileData) == 0)
                bStatus = false;

            return {bStatus};
        }

        friend bool operator==(const It& l, const It& r) { return l.bStatus == r.bStatus; }
        friend bool operator!=(const It& l, const It& r) { return l.bStatus != r.bStatus; }
    };

    It begin() { return {this, true}; }
    It end() { return {this, false}; }

    It begin() const { return {this, true}; }
    It end() const { return {this, false}; }
};

inline
Directory::Directory(const char* ntsPath)
{
    StringView sv = ntsPath;

    if (!sv.endsWith("/*"))
    {
        if (sv.size() < static_cast<ssize>(sizeof(m_aBuff) - 4))
        {
            strncpy(m_aBuff, ntsPath, sizeof(m_aBuff));

            if (sv.endsWith("/"))
            {
                m_aBuff[sv.size()] = '*';
            }
            else
            {
                m_aBuff[sv.size()] = '/';
                m_aBuff[sv.size() + 1] = '*';
            }
        }
    }
}

inline
Directory::operator bool() const
{
    return m_hFind != 0;
}

inline bool
Directory::close()
{
    int err = FindClose(m_hFind);

#ifndef NDEBUG
    if (err == 0)
        fprintf(stderr, "FindClose(): failed '%lu'\n", GetLastError());
#endif

    return err > 0;
}

#endif

} /* namespace adt */
