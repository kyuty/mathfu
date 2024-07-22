#pragma once

// // Suppress warnings in external header.
// #if defined(__GNUC__)
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wignored-qualifiers"
// #pragma GCC diagnostic ignored "-Wunused-parameter"
// #pragma GCC diagnostic ignored "-Wpedantic"
// #pragma GCC diagnostic ignored "-Wunused-value"
// #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// #pragma GCC diagnostic ignored "-Wnull-dereference"
// #pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
// #pragma GCC diagnostic ignored "-Wbitwise-instead-of-logical"
// #endif  // defined(__GNUC__)
//
// #if defined(__clang__)
// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wignored-qualifiers"
// #pragma clang diagnostic ignored "-Wunused-parameter"
// #pragma clang diagnostic ignored "-Wpedantic"
// #pragma clang diagnostic ignored "-Wunused-value"
// #pragma clang diagnostic ignored "-Wmissing-field-initializers"
// #pragma clang diagnostic ignored "-Wnull-dereference"
// #pragma clang diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
// #pragma clang diagnostic ignored "-Wbitwise-instead-of-logical"
// #endif  // defined(__clang__)
//
// xxxxx code
//     #include xxxx.h
//
// #if defined(__clang__)
// #pragma clang diagnostic pop
// #endif  // defined(__clang__)
//
// #if defined(__GNUC__)
// #pragma GCC diagnostic pop
// #endif  // defined(__GNUC__)

#define DISABLE_WARNING
#define ENABLE_WARNING

#if defined(__GNUC__)
#undef DISABLE_WARNING
#define DISABLE_WARNING \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wignored-qualifiers\"") \
_Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
_Pragma("GCC diagnostic ignored \"-Wpedantic\"") \
_Pragma("GCC diagnostic ignored \"-Wunused-value\"") \
_Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"") \
_Pragma("GCC diagnostic ignored \"-Wnull-dereference\"") \
_Pragma("GCC diagnostic ignored \"-Wdeprecated-copy-with-user-provided-copy\"") \
_Pragma("GCC diagnostic ignored \"-Wmacro-redefined\"") \
_Pragma("GCC diagnostic ignored \"-Wbitwise-instead-of-logical\"")
#endif

#if defined(__clang__)
#undef DISABLE_WARNING
#define DISABLE_WARNING \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wignored-qualifiers\"") \
_Pragma("clang diagnostic ignored \"-Wunused-parameter\"") \
_Pragma("clang diagnostic ignored \"-Wpedantic\"") \
_Pragma("clang diagnostic ignored \"-Wunused-value\"") \
_Pragma("clang diagnostic ignored \"-Wmissing-field-initializers\"") \
_Pragma("clang diagnostic ignored \"-Wnull-dereference\"") \
_Pragma("clang diagnostic ignored \"-Wdeprecated-copy-with-user-provided-copy\"") \
_Pragma("clang diagnostic ignored \"-Wmacro-redefined\"") \
_Pragma("clang diagnostic ignored \"-Wbitwise-instead-of-logical\"")
#endif

#if defined(__clang__)
#undef ENABLE_WARNING
#define ENABLE_WARNING \
_Pragma("clang diagnostic pop")
#endif

#if defined(__GNUC__)
#undef ENABLE_WARNING
#define ENABLE_WARNING \
_Pragma("GCC diagnostic pop")
#endif
