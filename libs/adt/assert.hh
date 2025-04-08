#pragma once

#include "types.hh"
#include "printDecl.hh" /* IWYU pragma: keep */

namespace adt
{

inline void
assertionFailed(const char* cnd, const char* msg, const char* file, int line, const char* func)
{
    char aBuff[256] {};
    print::toBuffer(aBuff, sizeof(aBuff) - 1,
        "[{}, {}: {}()] assertion( {} ) failed.\n(msg) '{}'\n",
        file, line, func, cnd, msg
    );

#if __has_include(<windows.h>)
    MessageBoxA(
        nullptr,
        aBuff,
        "Assertion failed",
        MB_ICONWARNING | MB_OK | MB_DEFBUTTON2
    );
#else
    fputs(aBuff, stderr);
    fflush(stderr);
#endif

    *(volatile int*)0 = 0; /* die */
}

} /* namespace adt */

#ifndef NDEBUG
    #define ADT_ASSERT(CND, ...)                                                                                       \
        do                                                                                                             \
        {                                                                                                              \
            if (!static_cast<bool>(CND))                                                                               \
            {                                                                                                          \
                char aMsgBuff[128] {};                                                                                 \
                adt::print::toBuffer(aMsgBuff, sizeof(aMsgBuff) - 1, __VA_ARGS__);                                     \
                adt::assertionFailed(#CND, aMsgBuff, ADT_LOGS_FILE, __LINE__, __func__);                               \
            }                                                                                                          \
        } while (0)
#else
    #define ADT_ASSERT(...) (void)0
#endif

#ifndef ADT_DISABLE_ASSERT_ALWAYS
    #define ADT_ASSERT_ALWAYS(CND, ...)                                                                                \
        do                                                                                                             \
        {                                                                                                              \
            if (!static_cast<bool>(CND))                                                                               \
            {                                                                                                          \
                char aMsgBuff[128] {};                                                                                 \
                adt::print::toBuffer(aMsgBuff, sizeof(aMsgBuff) - 1, __VA_ARGS__);                                     \
                adt::assertionFailed(#CND, aMsgBuff, ADT_LOGS_FILE, __LINE__, __func__);                               \
            }                                                                                                          \
        } while (0)
#else
    #define ADT_ASSERT_ALWAYS(...) (void)0
#endif
