#pragma once

#ifdef _WIN32
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT __attribute__((visibility("default")))
#endif // _WIN32