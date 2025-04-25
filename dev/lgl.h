#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define LITESPD_GL_LOG_ERROR(message)                                                                    \
    do {                                                                                                 \
        auto message___ = litespd::gl::lgi::format("[ERROR] %s(%d)\n%s\n", __FILE__, __LINE__, message); \
        fprintf(stderr, message___.c_str());                                                             \
        ::OutputDebugStringA(message___.c_str());                                                        \
    } while (false)
#define LITESPD_GL_LOG_WARNING(message)                                      \
    do {                                                                     \
        auto message___ = litespd::gl::lgi::format("[WARN_] %s\n", message); \
        fprintf(stderr, message___.c_str());                                 \
        ::OutputDebugStringA(message___.c_str());                            \
    } while (false)
#endif

#define LITESPD_GL_ENABLE_GLAD    1
#define LITESPD_GL_ENABLE_GLFW3   1
#define LITESPD_GL_ENABLE_GLM     1
#define LITESPD_GL_BACKTRACE(...) backtrace()
#include <litespd-gl/litespd-gl.h>

std::string backtrace();