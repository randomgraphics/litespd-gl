#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define LITESPD_GL_LOG_ERROR(message)                                   \
    do {                                                                \
        auto message___ = litespd::gl::format("[ERROR] %s\n", message); \
        fprintf(stderr, message___.c_str());                            \
        ::OutputDebugStringA(message___.c_str());                       \
    } while (false)
#define LITESPD_GL_LOG_WARNING(message)                                 \
    do {                                                                \
        auto message___ = litespd::gl::format("[WARN_] %s\n", message); \
        fprintf(stderr, message___.c_str());                            \
        ::OutputDebugStringA(message___.c_str());                       \
    } while (false)
#endif

// Include GLAD header.
#include <glad/glad.h>

// Include GLFW3 header.
#ifndef __ANDROID__
#define LITESPD_GL_ENABLE_GLFW3 1
#include <GLFW/glfw3.h>
#endif

#include <litespd-gl/litespd-gl.h>

std::string backtrace();