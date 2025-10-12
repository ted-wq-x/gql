#pragma once

#if defined(_WIN32)
#ifdef GQL_BUILD_SHARED
#define GQL_EXPORT __declspec(dllexport)
#elif defined(GQL_USE_SHARED)
#define GQL_EXPORT __declspec(dllimport)
#else
#define GQL_EXPORT
#endif
#else
#define GQL_EXPORT __attribute__((visibility("default")))
#endif