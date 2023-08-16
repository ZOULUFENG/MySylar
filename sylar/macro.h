#pragma once

#include "util.h"
#include <assert.h>
#include <string.h>

#if 1
#define SYLAR_ASSERT(x)                                                                    \
    {                                                                                      \
        if (!(x)) {                                                                        \
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << " ASSERTION: " #x                         \
                                              << "\nbacktrace:\n"                          \
                                              << sylar::BacktraceToString(100, 2, "    "); \
            assert(x);                                                                     \
        }                                                                                  \
    }

#endif

#if 1
#define SYLAR_ASSERT2(x, w)                                                                \
    {                                                                                      \
        if (!(x)) {                                                                        \
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x                          \
                                              << "\n"                                      \
                                              << w                                         \
                                              << "\nbacktrace:\n"                          \
                                              << sylar::BacktraceToString(100, 2, "    "); \
            assert(x);                                                                     \
        }                                                                                  \
    }
#endif
