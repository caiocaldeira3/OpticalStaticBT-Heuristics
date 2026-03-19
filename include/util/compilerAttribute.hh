#pragma once

// noinline
#if defined(__clang__) || defined(__GNUC__)
    #define NOINLINE __attribute__((__noinline__))
#else
    #define NOINLINE
#endif

// always inline
#if defined(__clang__) || defined(__GNUC__)
    #define ALWAYSINLINE inline __attribute__((__always_inline__))
#else
    #define ALWAYSINLINE inline
#endif

// flatten
#if defined(__GNUC__) && !defined(__clang__)
    #define FLATTEN_FUNC __attribute__((always_inline, flatten))
#else
    #define FLATTEN_FUNC ALWAYSINLINE
#endif