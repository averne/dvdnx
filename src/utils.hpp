#pragma once

#include <cstdio>

extern FILE *g_debug_file;

#define _STRINGIFY(x)        #x
#define  STRINGIFY(x)        _STRINGIFY(x)
#define _CONCATENATE(x1, x2) x1##x2
#define  CONCATENATE(x1, x2) _CONCATENATE(x1, x2)

#ifdef DEBUG
#   define LOG(fmt, ...) ({ \
        fprintf(g_debug_file, "%s: " fmt, __PRETTY_FUNCTION__ __VA_OPT__(,) __VA_ARGS__); \
        fflush(g_debug_file); \
    })
#else
#   define LOG(...) ({})
#endif
#define LOG32(v) LOG(STRINGIFY(v) " = %u\n", v)
#define LOG64(v) LOG(STRINGIFY(v) " = %lu\n", v)

#define TRY(x, cb) ({ \
	if (Result rc = (x); R_FAILED(rc)) { \
		LOG(STRINGIFY(x) " failed: %#x\n", rc); \
		({cb;}); \
	} \
})
#define TRY_GOTO(x, l)   TRY(x, goto l)
#define TRY_RETURN(x, v) TRY(x, return v)
#define TRY_FATAL(x)     TRY(x, fatalThrow(rc))

#define SERV_INIT(s, ...) TRY_FATAL(CONCATENATE(s, Initialize)(__VA_ARGS__))
#define SERV_EXIT(s, ...) CONCATENATE(s, Exit)(__VA_ARGS__)

#define ASSERT_SIZE(x, sz)        static_assert(sizeof(x) == (sz), "Wrong size in " STRINGIFY(x))
#define ASSERT_STANDARD_LAYOUT(x) static_assert(std::is_standard_layout_v<x>, STRINGIFY(x) " is not standard layout")
