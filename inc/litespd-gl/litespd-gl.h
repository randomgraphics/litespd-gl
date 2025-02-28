/*
MIT License

Copyright (c) 2023 randomgraphics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef LITESPD_GL_H_
#define LITESPD_GL_H_

/// Make sure GL/gl.h is included before this header.
#ifndef GL_VERSION_1_0
#error "GL/gl.h must be included before including litespd-gl.h"
#endif

/// A monotonically increasing number that uniquely identify the revision of the header.
#define LITESPD_GL_HEADER_REVISION 1

/// \def LITESPD_GL_NAMESPACE
/// Define the namespace of litespd-gl library.
#ifndef LITESPD_GL_NAMESPACE
#define LITESPD_GL_NAMESPACE litespd::gl
#endif

/// \def LITESPD_GL_ENABLE_DEBUG_BUILD
/// Set to non-zero value to enable debug build. Disabled by default.
#ifndef LITESPD_GL_ENABLE_DEBUG_BUILD
#define LITESPD_GL_ENABLE_DEBUG_BUILD 0
#endif

/// \def LITESPD_GL_ENABLE_GLFW3
/// Set to 1 to enable GLFW3 interation helpers. Disabled by default.
#ifndef LITESPD_GL_ENABLE_GLFW3
#define LITESPD_GL_ENABLE_GLFW3 0
#endif

/// \def LITESPD_GL_THROW
/// The macro to throw runtime exception.
/// \param errorString The error string to throw. Can be std::string or const char*.
#ifndef LITESPD_GL_THROW
#define LITESPD_GL_THROW(message) throw std::runtime_error(message)
#endif

/// \def LITESPD_GL_BACKTRACE
/// Define custom function to retrieve current call stack and store in std::string.
/// This macro is called when litespd-gl encounters critical error, to help
/// quickly identify the source of the error. The default implementation does
/// nothing but return empty string.
#ifndef LITESPD_GL_BACKTRACE
#define LITESPD_GL_BACKTRACE() std::string("You have to define LITESPD_GL_BACKTRACE to retrieve current call stack.")
#endif

/// \def LITESPD_GL_LOG_ERROR
/// The macro to log error message. The default implementation prints to stderr.
/// \paam message The error message to log. The type is const char *.
#ifndef LITESPD_GL_LOG_ERROR
#define LITESPD_GL_LOG_ERROR(message) fprintf(stderr, "[ ERROR ] %s\n", message)
#endif

/// \def LGI_LOGW
/// The macro to log warning message. The default implementation prints to stderr.
/// \param message The warning message to log. The type is const char *.
#ifndef LITESPD_GL_LOG_WARNING
#define LITESPD_GL_LOG_WARNING(message) fprintf(stderr, "[WARNING] %s\n", message)
#endif

/// \def LITESPD_GL_LOG_INFO
/// The macro to log informational message. The default implementation prints to stdout.
/// \param message The message to log. The type is const char *.
#ifndef LITESPD_GL_LOG_INFO
#define LITESPD_GL_LOG_INFO(message) fprintf(stdout, "%s\n", message)
#endif

/// \def LITESPD_GL_LOG_VERBOSE
/// The macro to log verbose log. The default implementation prints to stdout.
/// \param message The message to log. The type is const char *.
#ifndef LITESPD_GL_LOG_VERBOSE
#define LITESPD_GL_LOG_VERBOSE(message) fprintf(stdout, "[VERBOSE] %s\n", message)
#endif

/// \def LITESPD_GL_LOG_DEBUG
/// The macro to log debug message. The macro is ignored when LITESPD_GL_ENABLE_DEBUG_BUILD is 0
/// \param message The message to log. The type is const char *.
#ifndef LITESPD_GL_LOG_DEBUG
#define LITESPD_GL_LOG_DEBUG(message) fprintf(stdout, "[ DEBUG ] %s\n", message)
#endif

/// \def LITESPD_GL_ASSERT
/// The runtime assert macro for debug build only. This macro has no effect when
/// LITESPD_GL_ENABLE_DEBUG_BUILD is 0.
#ifndef LITESPD_GL_ASSERT
#define LITESPD_GL_ASSERT(expression, ...)                                       \
    if (!(expression)) {                                                           \
        auto errorMessage__ = LITESPD_GL_NAMESPACE::format(__VA_ARGS__);         \
        LGI_LOGE("Condition " #expression " not met. %s", errorMessage__.c_str()); \
        assert(false);                                                             \
    } else                                                                         \
        void(0)
#endif

// ---------------------------------------------------------------------------------------------------------------------
// include GLFW3 header if enabled and not included yet.
#if LITESPD_GL_ENABLE_GLFW3
#ifndef _glfw3_h_
#include <GLFW/glfw3.h>
#endif
#endif

// ---------------------------------------------------------------------------------------------------------------------
// include other standard/system headers

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <atomic>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <unordered_map>
#include <tuple>
#include <optional>
#include <algorithm>
#include <exception>
#include <functional>
#include <memory>

// ---------------------------------------------------------------------------------------------------------------------
// LGI stands for Litespd GL Implementation. Macros started with this prefix are reserved for internal use.

#define LGI_NO_COPY(T)                 \
    T(const T &)             = delete; \
    T & operator=(const T &) = delete;

#define LGI_NO_MOVE(T)            \
    T(T &&)             = delete; \
    T & operator=(T &&) = delete;

#define LGI_NO_COPY_NO_MOVE(T) LGI_NO_COPY(T) LGI_NO_MOVE(T)

#define LGI_STR(x) LGI_STR_HELPER(x)

#define LGI_STR_HELPER(x) #x

#define LGI_LOGE(...) LITESPD_GL_LOG_ERROR(LITESPD_GL_NAMESPACE::format(__VA_ARGS__).c_str())
#define LGI_LOGW(...) LITESPD_GL_LOG_WARNING(LITESPD_GL_NAMESPACE::format(__VA_ARGS__).c_str())
#define LGI_LOGI(...) LITESPD_GL_LOG_INFO(LITESPD_GL_NAMESPACE::format(__VA_ARGS__).c_str())
#define LGI_LOGV(...) LITESPD_GL_LOG_VERBOSE(LITESPD_GL_NAMESPACE::format(__VA_ARGS__).c_str())
#if LITESPD_GL_ENABLE_DEBUG_BUILD
#define LGI_LOGD(...) LITESPD_GL_LOG_DEBUG(LITESPD_GL_NAMESPACE::format(__VA_ARGS__).c_str())
#else
#define LGI_LOGD(...) void(0)
#endif

#define LGI_THROW(...)                                                                                       \
    do {                                                                                                     \
        std::stringstream errorStream_;                                                                      \
        errorStream_ << __FILE__ << "(" << __LINE__ << "): " << LITESPD_GL_NAMESPACE::format(__VA_ARGS__); \
        auto errorString_ = errorStream_.str();                                                              \
        LGI_LOGE("%s", errorString_.data());                                                                 \
        LITESPD_GL_THROW(errorString_);                                                                    \
    } while (false)

#if LITESPD_GL_ENABLE_DEBUG_BUILD
// assert is enabled only in debug build
#define LGI_ASSERT LITESPD_GL_ASSERT
#else
#define LGI_ASSERT(...) ((void) 0)
#endif

#define LGI_REQUIRE(condition, ...)                                                    \
    do {                                                                               \
        if (!(condition)) {                                                            \
            auto errorMessage__ = LITESPD_GL_NAMESPACE::format(__VA_ARGS__);         \
            LGI_THROW("Condition " #condition " not met. %s", errorMessage__.c_str()); \
        }                                                                              \
    } while (false)

#define LGI_VK_REQUIRE(condition, ...)                                                     \
    do {                                                                                   \
        if (VK_SUCCESS != (VkResult) (condition)) {                                        \
            auto errorMessage__ = LITESPD_GL_NAMESPACE::format(__VA_ARGS__);             \
            LGI_THROW("Vulkan fuction " #condition " failed. %s", errorMessage__.c_str()); \
        }                                                                                  \
    } while (false)

// Check C++ standard
#ifdef _MSC_VER
#if _MSVC_LANG < 201703L
#error "C++17 is required"
#elif _MSVC_LANG < 202002L
#define LGI_CXX_STANDARD 17
#else
#define LGI_CXX_STANDARD 20
#endif
#else
#if __cplusplus < 201703L
#error "c++17 or higher is required"
#elif __cplusplus < 202002L
#define LGI_CXX_STANDARD 17
#else
#define LGI_CXX_STANDARD 20
#endif
#endif

namespace LITESPD_GL_NAMESPACE {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nonstandard extension used: nameless struct/union
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace LITESPD_GL_NAMESPACE

#endif // LITESPD_GL_H

#ifdef LITESPD_GL_IMPLEMENTATION
#include "litespd-gl.cpp"
#endif
