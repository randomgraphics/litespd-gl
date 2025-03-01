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

#undef LITESPD_GL_IMPLEMENTATION
#include "litespd-gl.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
// #ifdef __clang__
// #pragma GCC diagnostic ignored "-Wnullability-completeness"
// #endif
// #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// #pragma GCC diagnostic ignored "-Wunused-parameter"
// #pragma GCC diagnostic ignored "-Wunused-variable"
// #pragma GCC diagnostic ignored "-Wtype-limits"
// #pragma GCC diagnostic ignored "-Wformat"
// #pragma GCC diagnostic ignored "-Wundef"
// #pragma GCC diagnostic ignored "-Wconversion"
// #pragma GCC diagnostic ignored "-Wparentheses"
// #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

#ifdef _WIN32
extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#endif

namespace LITESPD_GL_NAMESPACE {

namespace lgl_details {} // namespace lgl_details

#include <stb_image.h>
#include <stb_image_write.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <atomic>
#include <stack>

#if LITESPD_GL_ENABLE_DEBUG_BUILD && LITESPD_GL_ENABLE_GLAD
// -----------------------------------------------------------------------------
//
static void initializeOpenGLDebugRuntime() {
    struct OGLDebugOutput {
        static const char * source2String(GLenum source) {
            switch (source) {
            case GL_DEBUG_SOURCE_API_ARB:
                return "GL API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
                return "Window System";
            case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
                return "Shader Compiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
                return "Third Party";
            case GL_DEBUG_SOURCE_APPLICATION_ARB:
                return "Application";
            case GL_DEBUG_SOURCE_OTHER_ARB:
                return "Other";
            default:
                return "INVALID_SOURCE";
            }
        }

        static const char * type2String(GLenum type) {
            switch (type) {
            case GL_DEBUG_TYPE_ERROR_ARB:
                return "Error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
                return "Deprecation";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
                return "Undefined Behavior";
            case GL_DEBUG_TYPE_PORTABILITY_ARB:
                return "Portability";
            case GL_DEBUG_TYPE_PERFORMANCE_ARB:
                return "Performance";
            case GL_DEBUG_TYPE_OTHER_ARB:
                return "Other";
            default:
                return "INVALID_TYPE";
            }
        }

        static const char * severity2String(GLenum severity) {
            switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH_ARB:
                return "High";
            case GL_DEBUG_SEVERITY_MEDIUM_ARB:
                return "Medium";
            case GL_DEBUG_SEVERITY_LOW_ARB:
                return "Low";
            default:
                return "INVALID_SEVERITY";
            }
        }

        static void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                               GLsizei, // length,
                                               const GLchar * message,
                                               const void *) // userParam)
        {
            // Determine log level
            bool error_  = false;
            bool warning = false;
            bool info    = false;
            switch (type) {
            case GL_DEBUG_TYPE_ERROR_ARB:
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
                error_ = true;
                break;

            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            case GL_DEBUG_TYPE_PORTABILITY:
                switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH_ARB:
                case GL_DEBUG_SEVERITY_MEDIUM_ARB:
                    warning = true;
                    break;
                case GL_DEBUG_SEVERITY_LOW_ARB:
                    break;
                default:
                    error_ = true;
                    break;
                }
                break;

            case GL_DEBUG_TYPE_PERFORMANCE_ARB:
                switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH_ARB:
                    warning = true;
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM_ARB: // shader recompiliation, buffer data read back.
                case GL_DEBUG_SEVERITY_LOW_ARB:
                    break; // verbose: performance warnings from redundant state changes
                default:
                    error_ = true;
                    break;
                }
                break;

            case GL_DEBUG_TYPE_OTHER_ARB:
                switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH_ARB:
                    error_ = true;
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM_ARB:
                    warning = true;
                    break;
                case GL_DEBUG_SEVERITY_LOW_ARB:
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                    break; // verbose
                default:
                    error_ = true;
                    break;
                }
                break;

            default:
                error_ = true;
                break;
            }

            std::string s = formatString("(id=[%d] source=[%s] type=[%s] severity=[%s]): %s\n%s", id, source2String(source), type2String(type),
                                         severity2String(severity), message, dumpCallStack().c_str());
            if (error_)
                LGI_LOGE("[GL ERROR] %s", s.c_str());
            else if (warning)
                LGI_LOGW("[GL WARNING] %s", s.c_str());
            else if (info)
                LGI_LOGI("[GL INFO] %s", s.c_str());
        }
    };

    if (GLAD_GL_KHR_debug) {
        GLCHK(glDebugMessageCallback(&OGLDebugOutput::messageCallback, nullptr));
        GLCHK(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    } else if (GLAD_GL_ARB_debug_output) {
        GLCHK(glDebugMessageCallbackARB(&OGLDebugOutput::messageCallback, nullptr));
        // enable all messages
        GLCHK(glDebugMessageControlARB(GL_DONT_CARE, // source
                                       GL_DONT_CARE, // type
                                       GL_DONT_CARE, // severity
                                       0,            // count
                                       nullptr,      // ids
                                       GL_TRUE));
    }
}

// -----------------------------------------------------------------------------
//
static void printGLInfo(bool printExtensionList) {
    std::stringstream info;

    // vendor and version info.
    const char * vendor   = (const char *) glGetString(GL_VENDOR);
    const char * version  = (const char *) glGetString(GL_VERSION);
    const char * renderer = (const char *) glGetString(GL_RENDERER);
    const char * glsl     = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
    GLint        maxsls = -1, maxslsFast = -1;
    if (GLAD_GL_EXT_shader_pixel_local_storage) {
        glGetIntegerv(GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_SIZE_EXT, &maxsls);
        glGetIntegerv(GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT, &maxslsFast);
    }
    info << "\n\n"
            "===================================================\n"
            "        OpenGL Implementation Information\n"
            "---------------------------------------------------\n"
            "               OpenGL vendor : "
         << vendor
         << "\n"
            "              OpenGL version : "
         << version
         << "\n"
            "             OpenGL renderer : "
         << renderer
         << "\n"
            "                GLSL version : "
         << glsl
         << "\n"
            "       Max FS uniform blocks : "
         << getInt(GL_MAX_FRAGMENT_UNIFORM_BLOCKS)
         << "\n"
            "      Max uniform block size : "
         << getInt(GL_MAX_UNIFORM_BLOCK_SIZE) * 4
         << " bytes\n"
            "           Max texture units : "
         << getInt(GL_MAX_TEXTURE_IMAGE_UNITS)
         << "\n"
            "    Max array texture layers : "
         << getInt(GL_MAX_ARRAY_TEXTURE_LAYERS)
         << "\n"
            "       Max color attachments : "
         << getInt(GL_MAX_COLOR_ATTACHMENTS)
         << "\n"
            "           Max SSBO binding  : "
         << getInt(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS)
         << "\n"
            "         Max SSBO FS blocks  : "
         << getInt(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS)
         << "\n"
            "        Max SSBO block size  : "
         << getInt(GL_MAX_SHADER_STORAGE_BLOCK_SIZE) * 4
         << " bytes\n"
            "       Max CS WorkGroup size : "
         << getInt(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0) << "," << getInt(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1) << "," << getInt(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2)
         << "\n"
            "      Max CS WorkGroup count : "
         << getInt(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0) << "," << getInt(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1) << "," << getInt(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2)
         << "\n"
            "    Max shader local storage : total="
         << maxsls << ", fast=" << maxslsFast << "\n";

    if (printExtensionList) {
        info << "---------------------------------------------------\n";
        std::vector<std::string> extensions;
        GLint                    n = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &n);
        for (int i = 0; i < n; ++i) { extensions.push_back((const char *) glGetStringi(GL_EXTENSIONS, i)); }
        std::sort(extensions.begin(), extensions.end());
        for (int i = 0; i < n; ++i) { info << "    " << extensions[i] << "\n"; }
    }

    info << "===================================================\n";

    LGI_LOGI(info.str().c_str());
}

// -----------------------------------------------------------------------------
//
void initGlad(bool printExtensionList) {
    static std::atomic_bool initialized = false;
    if (initialized.exchange(true)) return;
#ifdef __ANDROID__
    typedef void * (*GetProcAddress)(const char * name);
    auto gpa = (GetProcAddress) dlsym(nullptr, "eglGetProcAddress");
    // Global look-up can fail if the application didn't open/call to this function
    // yet, it somehow happen in 32-bit mode so we can fallback to attempt reading
    // libEGL.so directly
    if (!gpa) {
        auto handle = dlopen("libEGL.so", RTLD_LAZY | RTLD_LOCAL);
        gpa         = (GetProcAddress) dlsym(handle, "eglGetProcAddress");
    }
    LGI_CHK(gpa);
    LGI_CHK(gladLoadGLES2Loader(gpa));
#else
    LGI_CHK(gladLoadGL());
#endif

#if _DEBUG
    initializeOpenGLDebugRuntime();
#endif

    printGLInfo(printExtensionList);
}

#endif

// -----------------------------------------------------------------------------
//
void TextureObject::attach(GLenum target, GLuint id) {
    cleanup();
    _owned       = false;
    _desc.target = target;
    _desc.id     = id;
    bind(0);

    glGetTexLevelParameteriv(_desc.target, 0, GL_TEXTURE_WIDTH, (GLint *) &_desc.width);
    LGI_ASSERT(_desc.width);

    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, (GLint *) &_desc.height);
    LGI_ASSERT(_desc.height);

    // determine depth
    switch (target) {
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_3D:
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH, (GLint *) &_desc.depth);
        LGI_ASSERT(_desc.depth);
        break;
    case GL_TEXTURE_CUBE_MAP:
        _desc.depth = 6;
        break;
    default:
        _desc.depth = 1;
        break;
    }

    GLint maxLevel;
    glGetTexParameteriv(target, GL_TEXTURE_MAX_LEVEL, &maxLevel);
    _desc.mips = (uint32_t) maxLevel + 1;

    int internalFormat = 0;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    _desc.format = fromGLInternalFormat(internalFormat);

    unbind();
}

// -----------------------------------------------------------------------------
//
void TextureObject::allocate2D(GLint internalFormat, size_t w, size_t h, size_t m) {
    cleanup();
    _desc.target = GL_TEXTURE_2D;
    _desc.format = f;
    _desc.width  = (uint32_t) w;
    _desc.height = (uint32_t) h;
    _desc.depth  = (uint32_t) 1;
    _desc.mips   = (uint32_t) m;
    _owned       = true;
    GLCHK(glGenTextures(1, &_desc.id));
    GLCHK(glBindTexture(_desc.target, _desc.id));
    applyDefaultParameters();
    GLCHK(glTexStorage2D(_desc.target, (GLsizei) _desc.mips, internalFormat, (GLsizei) _desc.width, (GLsizei) _desc.height));
    GLCHK(glBindTexture(_desc.target, 0));
}

// -----------------------------------------------------------------------------
//
void TextureObject::allocate2DArray(GLint internalFormat, size_t w, size_t h, size_t l, size_t m) {
    cleanup();
    _desc.target = GL_TEXTURE_2D_ARRAY;
    _desc.format = f;
    _desc.width  = (uint32_t) w;
    _desc.height = (uint32_t) h;
    _desc.depth  = (uint32_t) l;
    _desc.mips   = (uint32_t) m;
    _owned       = true;
    GLCHK(glGenTextures(1, &_desc.id));
    GLCHK(glBindTexture(_desc.target, _desc.id));
    applyDefaultParameters();
    GLCHK(glTexStorage3D(_desc.target, (GLsizei) _desc.mips, internalFormat, (GLsizei) _desc.width, (GLsizei) _desc.height, (GLsizei) _desc.depth));
    GLCHK(glBindTexture(_desc.target, 0));
}

// -----------------------------------------------------------------------------
//
void TextureObject::allocateCube(GLint internalFormat, size_t w, size_t m) {
    cleanup();
    _desc.target = GL_TEXTURE_CUBE_MAP;
    _desc.format = f;
    _desc.width  = (uint32_t) w;
    _desc.height = (uint32_t) w;
    _desc.depth  = 6;
    _desc.mips   = (uint32_t) m;
    _owned       = true;
    GLCHK(glGenTextures(1, &_desc.id));
    GLCHK(glBindTexture(GL_TEXTURE_CUBE_MAP, _desc.id));
    applyDefaultParameters();
    const auto & cd = jedi::getColorFormatDesc(_desc.format);
    GLCHK(glTexStorage2D(GL_TEXTURE_CUBE_MAP, (GLsizei) _desc.mips, internalFormat, (GLsizei) _desc.width, (GLsizei) _desc.width));
    GLCHK(glBindTexture(_desc.target, 0));
}

void TextureObject::applyDefaultParameters() {
    LGI_ASSERT(_desc.width > 0);
    LGI_ASSERT(_desc.height > 0);
    LGI_ASSERT(_desc.depth > 0);
    LGI_ASSERT(_desc.mips > 0);
    GLCHK(glTexParameteri(_desc.target, GL_TEXTURE_BASE_LEVEL, 0));
    GLCHK(glTexParameteri(_desc.target, GL_TEXTURE_MAX_LEVEL, _desc.mips - 1));
    GLCHK(glTexParameteri(_desc.target, GL_TEXTURE_MIN_FILTER, _desc.mips > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST));
    GLCHK(glTexParameteri(_desc.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(_desc.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCHK(glTexParameteri(_desc.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

// -----------------------------------------------------------------------------
//
void TextureObject::setPixels(size_t level, size_t x, size_t y, size_t w, size_t h, size_t rowPitchInBytes, const void * pixels) const {
    if (empty()) return;
    GLCHKDBG(glBindTexture(_desc.target, _desc.id));
    auto & cf = getColorFormatDesc(_desc.format);
    LGI_ASSERT(0 == (rowPitchInBytes * 8 % cf.bits));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (int) (rowPitchInBytes * 8 / cf.bits));
    GLCHKDBG(glTexSubImage2D(_desc.target, (GLint) level, (GLint) x, (GLint) y, (GLsizei) w, (GLsizei) h, cf.glFormat, cf.glType, pixels));
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    GLCHK(;);
}

void TextureObject::setPixels(size_t layer, size_t level, size_t x, size_t y, size_t w, size_t h, size_t rowPitchInBytes, const void * pixels) const {
    if (empty()) return;

    GLCHKDBG(glBindTexture(_desc.target, _desc.id));
    auto & cf = getColorFormatDesc(_desc.format);
    LGI_ASSERT(0 == (rowPitchInBytes * 8 % cf.bits));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (int) (rowPitchInBytes * 8 / cf.bits));

    GLCHKDBG(glTexSubImage3D(_desc.target, (GLint) level, (GLint) x, (GLint) y, (GLint) layer, (GLsizei) w, (GLsizei) h, 1, cf.glFormat, cf.glType, pixels));

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    GLCHK(;);
}

// -----------------------------------------------------------------------------
//
jedi::ManagedRawImage TextureObject::getBaseLevelPixels() const {
    if (empty()) return {};
    jedi::ManagedRawImage image(ImageDesc(_desc.format, _desc.width, _desc.height, _desc.depth));
#ifdef __ANDROID__
    if (_desc.target == GL_TEXTURE_2D) {
        GLuint _frameBuffer = 0;
        glGenFramebuffers(1, &_frameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _desc.target, _desc.id, 0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, _desc.width, _desc.height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        GLCHK(;);
    } else {
        LGI_LOGE("read texture 2D array pixels is not implemented on android yet.");
    }
#else
    auto & cf = getColorFormatDesc(_desc.format);
    glPixelStorei(GL_UNPACK_ALIGNMENT, image.alignment());
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (int) image.pitch() * 8 / (int) cf.bits);
    glBindTexture(_desc.target, _desc.id);
    glGetTexImage(_desc.target, 0, cf.glFormat, cf.glType, image.data());
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    GLCHK(;);
#endif
    return image;
}

// -----------------------------------------------------------------------------
//
void SimpleFBO::cleanup() {
    for (int i = 0; i < COLOR_BUFFER_COUNT; ++i) {
        if (_colors[i].texture) {
            glDeleteTextures(1, &_colors[i].texture);
            _colors[i].texture = 0;
        }
    }
    if (_depth) glDeleteTextures(1, &_depth), _depth = 0;
    for (auto & m : _mips) {
        if (m.fbo) glDeleteFramebuffers(1, &m.fbo), m.fbo = 0;
    }
    _mips.clear();
}

// -----------------------------------------------------------------------------
//
void SimpleFBO::allocate(uint32_t w, uint32_t h, uint32_t levels, const ColorFormat * colorFormats) {
    GLCHK(;); // make sure there's no preexisting conditions.

    cleanup(); // release existing buffers.

    LGI_ASSERT(w > 0 && h > 0);

    // create mips array
    while (w > 0 && h > 0 && (0 == levels || _mips.size() < levels)) {
        _mips.push_back({w, h, 0});
        GLCHK(glGenFramebuffers(1, &_mips.back().fbo));
        if (w > 0) w >>= 1;
        if (h > 0) h >>= 1;
    };
    levels = (uint32_t) _mips.size();

    const auto minfilter = _mips.size() > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;

    if (colorFormats) {
        _colorTextureTarget   = GL_TEXTURE_2D;
        GLenum drawBuffers[8] = {};
        for (int i = 0; i < COLOR_BUFFER_COUNT; ++i) {
            GLCHK(glGenTextures(1, &_colors[i].texture));
            auto cf                     = colorFormats[i];
            _colors[i].format           = cf;
            const GLenum internalFormat = getColorFormatDesc(cf).glInternalFormat;
            GLCHK(glBindTexture(GL_TEXTURE_2D, _colors[i].texture));
            GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
            GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) (levels - 1)));
            GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter));
            GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GLCHK(glTexStorage2D(GL_TEXTURE_2D, levels, internalFormat, _mips[0].width, _mips[0].height));
            for (int l = 0; l < (int) _mips.size(); ++l) {
                auto & m = _mips[l];
                GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, m.fbo));
                GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER, (GLenum) (GL_COLOR_ATTACHMENT0 + i), GL_TEXTURE_2D, _colors[i].texture, l));
            }
            drawBuffers[i] = (GLenum) (GL_COLOR_ATTACHMENT0 + i);
        }
        for (auto & m : _mips) {
            GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, m.fbo));
            GLCHK(glDrawBuffers(COLOR_BUFFER_COUNT, drawBuffers));
        }
    } else {
        GLenum none = GL_NONE;
        for (auto & m : _mips) {
            GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, m.fbo));
            GLCHK(glDrawBuffers(1, &none));
        }
    }

    if (HAS_DEPTH) {
        // depth (use texture instead of renderbuffer, since it is very likely that we'll need
        // to read depth data in the future.)
        GLCHK(glGenTextures(1, &_depth));
        GLCHK(glBindTexture(GL_TEXTURE_2D, _depth));
        // For OpenGL ES, depth texture works only when the texture is defined with
        // glTexImage2D() using GL_DEPTH_COMPONENT as the internal format, as specified
        // in OES_depth_texture extension.
        // On OpenGL CORE profile though, glTexStorage2D() works just fine.
        // GLCHK(glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT, w, h));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) (_mips.size() - 1)));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        for (int l = 0; l < (int) _mips.size(); ++l) {
            auto & m = _mips[l];
            GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, m.fbo));
            // Note: switching to 16-bit (GL_UNSIGNED_SHORT) depth buffer gives about 3ms performance boost.
            GLCHK(glTexImage2D(GL_TEXTURE_2D, l, GL_DEPTH_COMPONENT, m.width, m.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr));
            GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depth, l));
        }
    }

    // todo: stencil?

    // make sure the FBO is ready to use.
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    LGI_CHK(GL_FRAMEBUFFER_COMPLETE == status);
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

// -----------------------------------------------------------------------------
//
static void SaveImageToPNG(const float * pixels, uint32_t w, uint32_t h, uint32_t channels, const std::string & filepath) {
    // convert data to RGB8
    size_t               numPixels = w * h;
    std::vector<uint8_t> rgb8(numPixels * 3);
    for (size_t i = 0; i < numPixels; ++i) {
        uint8_t *     d = &rgb8[i * 3];
        const float * s = &pixels[i * channels];
        for (size_t c = 0; c < 3; ++c) {
            if (c < channels) {
                auto f = s[c] * 255.0f;
                if (f < 0.f)
                    d[c] = 0;
                else if (f > 255.f)
                    d[c] = 255;
                else
                    d[c] = (uint8_t) f;
            } else {
                d[c] = 0;
            }
        }
    }
    LGI_LOGI("Save texture content to %s", filepath.c_str());
    stbi_write_png(filepath.c_str(), (int) w, (int) h, 3, rgb8.data(), 0);
}

// -----------------------------------------------------------------------------
//
static void saveTextureToFile(uint32_t w, uint32_t h, uint32_t channels, GLenum target, GLenum format, GLenum type, const std::string & filepath) {
    LGI_LOGI("Save texture content to %s", filepath.c_str());
    std::vector<float> pixels(w * h * channels);
    GLCHK(glGetTexImage(target, 0, format, type, pixels.data()));

    // Flip the image vertically, since OpenGL texture is bottom-up.
    std::vector<float> flipped(w * h * channels);
    for (size_t i = 0; i < h; ++i) {
        const float * s = &pixels[w * channels * (h - i - 1)];
        float *       d = &flipped[w * channels * i];
        memcpy(d, s, w * channels * sizeof(float));
    }

    std::ofstream file(filepath, std::ofstream::binary);
    if (!file.good()) {
        LGI_LOGE("Failed to open file %s for writing", filepath.c_str());
        return;
    }
    struct FileHeader {
        char     FILE_TAG[8];
        uint32_t w, h, channels;
        GLenum   type;
    } header = {{'F', 'T', 'L', 'I', 'M', 'A', 'G', 'E'}, w, h, channels, type};
    file.write((const char *) &header, sizeof(header));
    file.write((const char *) flipped.data(), flipped.size() * sizeof(float));
    file.close();

    // Also save to png file just for easy previewing.
    SaveImageToPNG(flipped.data(), w, h, channels, filepath + ".png");
}

// -----------------------------------------------------------------------------
//
void SimpleFBO::saveColorToFile(uint32_t rt, const std::string & filepath) const {
    bindColorAsTexture(rt, 0);
    saveTextureToFile(_mips[0].width, _mips[0].height, 4, _colorTextureTarget, GL_RGBA, GL_FLOAT, filepath);
}

// -----------------------------------------------------------------------------
//
void SimpleFBO::saveDepthToFile(const std::string & filepath) const {
    bindDepthAsTexture(0);
    saveTextureToFile(_mips[0].width, _mips[0].height, 1, _colorTextureTarget, GL_DEPTH_COMPONENT, GL_FLOAT, filepath);
}

// -----------------------------------------------------------------------------
//
void CubeFBO::allocate(uint32_t w, uint32_t levels, ColorFormat cf) {
    cleanup(); // release existing buffers.

    LGI_ASSERT(w > 0);

    // create mips array
    while (w > 0 && (0 == levels || _mips.size() < levels)) {
        _mips.push_back({w, {}});
        GLCHK(glGenFramebuffers(6, _mips.back().fbo));
        w >>= 1;
    };

    const auto minfilter = _mips.size() > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;

    const GLenum internalFormat = getColorFormatDesc(cf).glInternalFormat;
    if (ColorFormat::NONE != cf) {
        GLCHK(glGenTextures(1, &_color));
        GLCHK(glBindTexture(GL_TEXTURE_CUBE_MAP, _color));
        GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0));
        GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, (GLint) _mips.size() - 1));
        GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minfilter));
        GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLCHK(glTexStorage2D(GL_TEXTURE_CUBE_MAP, (GLsizei) _mips.size(), internalFormat, (GLsizei) _mips[0].width, (GLsizei) _mips[0].width));
        for (int l = 0; l < (int) _mips.size(); ++l) {
            const auto & m = _mips[l];
            for (int i = 0; i < 6; ++i) {
                GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, m.fbo[i]));
                GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _color, l));
            }
        }
    } else {
        GLenum none = GL_NONE;
        for (auto & m : _mips) {
            for (int i = 0; i < 6; ++i) {
                GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, m.fbo[i]));
                GLCHK(glDrawBuffers(1, &none));
            }
        }
    }

    // depth (use texture instead of renderbuffer, since it is very likely that we'll need
    // to read depth data in the future.)
    GLCHK(glGenTextures(1, &_depth));
    GLCHK(glBindTexture(GL_TEXTURE_CUBE_MAP, _depth));
    // For OpenGL ES, depth texture works only when the texture is defined with
    // glTexImage2D() using GL_DEPTH_COMPONENT as the internal format, as specified
    // in OES_depth_texture extension.
    // On OpenGL CORE profile though, glTexStorage2D() works just fine.
    // GLCHK(glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT, w, h));
    GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0));
    GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, (GLint) _mips.size() - 1));
    GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minfilter));
    GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    for (int l = 0; l < (int) _mips.size(); ++l) {
        const auto & m = _mips[l];
        for (int i = 0; i < 6; ++i) {
            GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, m.fbo[i]));
            GLCHK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, l, GL_DEPTH_COMPONENT, m.width, m.width, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr));
            GLCHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _depth, l));
        }
    }

    // todo: stencil?

    // make sure the FBO is ready to use.
    for (int l = 0; l < (int) _mips.size(); ++l) {
        const auto & m = _mips[l];
        for (int i = 0; i < 6; ++i) {
            GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, m.fbo[i]));
            auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            LGI_CHK(GL_FRAMEBUFFER_COMPLETE == status);
        }
    }

    // done
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void DebugSSBO::printLastResult() const {
#if DEBUG_SSBO_ENABLED
    if (!counter) return;
    auto count    = std::min<size_t>((*counter), buffer.size() - 1);
    auto dataSize = sizeof(float) * (count + 1);
    if (0 != memcmp(buffer.data(), printed.data(), dataSize)) {
        memcpy(printed.data(), buffer.data(), dataSize);
        std::stringstream ss;
        ss << "count = " << *counter << " [";
        for (size_t i = 0; i < count; ++i) {
            auto value = printed[i + 1];
            if (std::isnan(value))
                ss << std::endl;
            else
                ss << value << ", ";
        }
        ss << "]";
        LGI_LOGI("%s", ss.str().c_str());
    }
#endif
}

// -----------------------------------------------------------------------------
//
ScreenQuad & ScreenQuad::allocate() {
    // Cleanup previous array if any.
    cleanup();

    // Create new array.
    GLCHK(glGenVertexArrays(1, &va));
    GLCHK(glBindVertexArray(va));
    vb.allocate<Vertex>(6, nullptr);
    GLCHK(vb.bind());
    GLCHK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *) 0));
    GLCHK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *) offsetof(Vertex, u)));
    GLCHK(glEnableVertexAttribArray(0));
    GLCHK(glEnableVertexAttribArray(1));
    GLCHK(glBindVertexArray(0)); // unbind

    // update the buffer with default value.
    update();

    return *this;
}

// -----------------------------------------------------------------------------
//
ScreenQuad & ScreenQuad::cleanup() {
    vb.cleanup();

    // If we actually have a vertex array to cleanup.
    if (va) {
        // Delete the vertex array.
        glDeleteVertexArrays(1, &va);

        // Reset this to mark it as cleaned up.
        va = 0;
    }
    GLCHK(;);
    return *this;
}

// -----------------------------------------------------------------------------
//
ScreenQuad & ScreenQuad::update(const glm::vec4 pos, const glm::vec4 & uv) {
    const Vertex vertices[] = {
        {pos.x, pos.w, uv.x, uv.w}, {pos.y, pos.w, uv.y, uv.w}, {pos.x, pos.z, uv.x, uv.z},
        {pos.x, pos.z, uv.x, uv.z}, {pos.y, pos.w, uv.y, uv.w}, {pos.y, pos.z, uv.y, uv.z},
    };
    vb.update(vertices, 0, 6);
    return *this;
}

// -----------------------------------------------------------------------------
//
static const char * shaderType2String(GLenum shaderType) {
    switch (shaderType) {
    case GL_VERTEX_SHADER:
        return "vertex";
    case GL_FRAGMENT_SHADER:
        return "fragment";
    case GL_COMPUTE_SHADER:
        return "compute";
    default:
        return "";
    }
}

// -----------------------------------------------------------------------------
static std::string addLineCount(const std::string & in) {
    std::stringstream ss;
    ss << "(  1) : ";
    int line = 1;
    for (auto ch : in) {
        if ('\n' == ch)
            ss << formatString("\n(%3d) : ", ++line);
        else
            ss << ch;
    }
    return ss.str();
}

// -----------------------------------------------------------------------------
//
GLuint loadShaderFromString(const char * source, size_t length, GLenum shaderType, const char * optionalFilename) {
    if (!source) return 0;
    const char * sources[] = {source};
    if (0 == length) length = strlen(source);
    GLint sizes[] = {(GLint) length};
    auto  shader  = glCreateShader(shaderType);
    glShaderSource(shader, 1, sources, sizes);
    glCompileShader(shader);

    // check for shader compile errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (GL_TRUE != success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        glDeleteShader(shader);
        LGI_LOGE("\n================== Failed to compile %s shader '%s' ====================\n"
                 "%s\n"
                 "\n============================= GLSL shader source ===============================\n"
                 "%s\n"
                 "\n================================================================================\n",
                 shaderType2String(shaderType), optionalFilename ? optionalFilename : "<no-name>", infoLog, addLineCount(source).c_str());
        return 0;
    }

    // done
    LGI_ASSERT(shader);
    return shader;
}

// -----------------------------------------------------------------------------
//
GLuint linkProgram(const std::vector<GLuint> & shaders, const char * optionalProgramName) {
    auto program = glCreateProgram();
    for (auto s : shaders)
        if (s) glAttachShader(program, s);
    glLinkProgram(program);
    for (auto s : shaders)
        if (s) glDetachShader(program, s);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        glDeleteProgram(program);
        LGI_LOGE("Failed to link program %s:\n%s", optionalProgramName ? optionalProgramName : "", infoLog);
        return 0;
    }

    // Enable the following code to dump GL program binary to disk.
#if 0
    if (optionalProgramName) {
        std::string outfilename = std::string(optionalProgramName) + ".bin";
        std::ofstream fs;
        fs.open(outfilename);
        if (fs.good()) {
            std::vector<uint8_t> buffer(1024 * 1024 * 1024); // allocate 1MB buffer.
            GLsizei len;
            GLenum dummyFormat;
            GLCHK(glGetProgramBinary(program, (GLsizei)buffer.size(), &len, &dummyFormat, buffer.data()));
            fs.write((const char*)buffer.data(), len);
        }
    }
#endif

    // done
    LGI_ASSERT(program);
    return program;
}

// -----------------------------------------------------------------------------
//
bool SimpleSprite::init() {
    cleanup();
    const char * vscode = R"(#version 320 es
        layout (location = 0) in vec2 a_position;
        layout (location = 1) in vec2 a_uv;
        out vec2 v_uv;
        void main()
        {
            gl_Position = vec4(a_position, 0.0, 1.0);
            v_uv = a_uv;
        }
    )";
    const char * pscode = R"(#version 320 es
        precision mediump float;
        layout(binding = 0) uniform sampler2D u_tex0;
        in vec2 v_uv;
        out vec4 o_color;
        void main()
        {
            o_color = texture(u_tex0, v_uv).xyzw;
        }
    )";
    if (!_program.loadVsPs(vscode, pscode)) return false;
    _tex0Binding = _program.getUniformBinding("u_tex0");

    _quad.allocate();

    // create sampler object
    glGenSamplers(1, &_sampler);
    glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameterf(_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // done
    return true;
}

// -----------------------------------------------------------------------------
//
void SimpleSprite::cleanup() {
    _program.cleanup();
    _quad.cleanup();
    if (_sampler) glDeleteSamplers(1, &_sampler), _sampler = 0;
}

// -----------------------------------------------------------------------------
//
void SimpleSprite::draw(GLuint texture, const glm::vec4 & pos, const glm::vec4 & uv) {
    _quad.update(pos, uv);
    _program.use();
    glActiveTexture(GL_TEXTURE0 + _tex0Binding);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(_tex0Binding, _sampler);
    _quad.draw();
}

// -----------------------------------------------------------------------------
//
bool SimpleTextureCopy::init() {
    const char * vscode = R"(#version 320 es
        out vec2 v_uv;
        void main()
        {
            const vec4 v[] = vec4[](
                vec4(-1., -1.,  0., 0.),
                vec4( 3., -1.,  2., 0.),
                vec4(-1.,  3.,  0., 2.));
            gl_Position = vec4(v[gl_VertexID].xy, 0., 1.);
            v_uv = v[gl_VertexID].zw;
        }
    )";
    const char * pscode = R"(
        #version 320 es
        precision mediump float;
        layout(binding = 0) uniform %s u_tex0;
        in vec2 v_uv;
        out vec4 o_color;
        void main()
        {
            o_color = texture(u_tex0, %s).xyzw;
        }
    )";

    // tex2d program
    {
        auto & prog2d = _programs[GL_TEXTURE_2D];
        auto   ps2d   = formatString(pscode, "sampler2D", "u_uv");
        if (!prog2d.program.loadVsPs(vscode, pscode)) return false;
        prog2d.tex0Binding = prog2d.program.getUniformBinding("u_tex0");
    }

    // tex2d array program
    {
        auto & prog2darray = _programs[GL_TEXTURE_2D_ARRAY];
        auto   ps2darray   = formatString(pscode, "sampler2DArray", "vec3(u_uv, 0.)");
        if (!prog2darray.program.loadVsPs(vscode, pscode)) return false;
        prog2darray.tex0Binding = prog2darray.program.getUniformBinding("u_tex0");
    }

    // create sampler object
    glGenSamplers(1, &_sampler);
    glSamplerParameteri(_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameterf(_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    _quad.allocate();
    glGenFramebuffers(1, &_fbo);

    GLCHK(;); // make sure we have no errors.
    return true;
}

// -----------------------------------------------------------------------------
//
void SimpleTextureCopy::cleanup() {
    _programs.clear();
    _quad.cleanup();
    if (_fbo) glDeleteFramebuffers(1, &_fbo), _fbo = 0;
    if (_sampler) glDeleteSamplers(1, &_sampler), _sampler = 0;
}

// -----------------------------------------------------------------------------
//
void SimpleTextureCopy::copy(const TextureSubResource & src, const TextureSubResource & dst) {
    // get destination texture size
    uint32_t dstw = 0, dsth = 0;
    glBindTexture(dst.target, dst.id);
    glGetTexLevelParameteriv(dst.target, dst.level, GL_TEXTURE_WIDTH, (GLint *) &dstw);
    glGetTexLevelParameteriv(dst.target, dst.level, GL_TEXTURE_HEIGHT, (GLint *) &dsth);

    // attach FBO to the destination texture
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    switch (dst.target) {
    case GL_TEXTURE_2D:
        glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst.id, dst.level);
        break;

    case GL_TEXTURE_2D_ARRAY:
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dst.id, dst.level, dst.z);
        break;

    default:
        // 3D or cube texture
        LGI_LOGE("unsupported destination texture target.");
        return;
    }
    constexpr GLenum drawbuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawbuffer);
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        LGI_LOGE("the frame buffer is not complete.");
        return;
    }

    // get the porgram based on source target
    auto & prog = _programs[src.target];
    if (0 == prog.program) {
        LGI_LOGE("unsupported source texture target.");
        return;
    }

    // do the copy
    prog.program.use();
    glActiveTexture(GL_TEXTURE0 + prog.tex0Binding);
    glBindTexture(src.target, src.id);
    glBindSampler(prog.tex0Binding, _sampler);
    glViewport(0, 0, dstw, dsth);
    _quad.draw();
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // done. make sure we are error clean.
    GLCHKDBG(;);
}

// -----------------------------------------------------------------------------
//
void GpuTimeElapsedQuery::stop() {
    if (_q.running()) {
        _q.end();
    } else {
        _q.getResult(_result);
    }
}

// -----------------------------------------------------------------------------
//
std::string GpuTimeElapsedQuery::print() const { return formatString("%s : %s"), name.c_str(), jedi::ns2str(duration()).c_str(); }

// -----------------------------------------------------------------------------
//
std::string GpuTimestamps::print(const char * ident) const {
    if (_marks.size() < 2) return {};
    std::stringstream ss;
    GLuint64          startTime = _marks[0].result;
    GLuint64          prevTime  = startTime;
    if (!ident) ident = "";
    if (0 == startTime) {
        ss << ident << "all timestamp queries are pending...\n";
    } else {

        auto getDuration = [](uint64_t a, uint64_t b) { return b >= a ? jedi::ns2str(b - a) : "  <n/a>"; };

        size_t maxlen = 0;
        for (size_t i = 1; i < _marks.size(); ++i) { maxlen = std::max(_marks[i].name.size(), maxlen); }
        for (size_t i = 1; i < _marks.size(); ++i) {
            auto current = _marks[i].result;
            if (0 == current) {
                ss << ident << "pending...\n";
                break;
            }
            // auto fromStart = current > startTime ? (current - startTime) : 0;
            auto delta = getDuration(prevTime, current);
            ss << ident << std::setw(maxlen) << std::left << _marks[i].name << std::setw(0) << " : " << delta << std::endl;
            prevTime = current;
        }
        ss << ident << "total = " << getDuration(_marks.front().result, _marks.back().result) << std::endl;
    }
    return ss.str();
}

// -----------------------------------------------------------------------------
//
#if 0
// This code path creates shared GL context using native Win32 API w/o using any 3rd party libraries.
// It is not currenty being used, but kept as reference.
#include <windows.h>
class RenderContext::Impl
{
    HWND _window = 0;
    HDC _dc = 0;
    HGLRC _rc = 0;

public:

    ~Impl()
    {
        destroy();
    }

    bool create(void *)
    {
        destroy();

        auto currentRC = wglGetCurrentContext();
        auto currentDC = wglGetCurrentDC();
        auto currentPF = GetPixelFormat(currentDC);
        PIXELFORMATDESCRIPTOR currentPfd;
        currentPfd.nSize = sizeof(currentPfd);
        if (0 == DescribePixelFormat(currentDC, currentPF, sizeof(currentPfd), &currentPfd)) {
            LGI_LOGE("Failed to get current PFD");
            return false;
        }

        // get class name
        auto className = "shared context class";

        WNDCLASSA wc = {};
        wc.lpfnWndProc    = (WNDPROC)&DefWindowProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = 0;
        wc.hInstance      = (HINSTANCE)GetModuleHandleW(nullptr);
        wc.hIcon          = LoadIcon (0, IDI_APPLICATION);
        wc.hCursor        = LoadCursor (0,IDC_ARROW);
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName   = 0;
        wc.lpszClassName  = className;
        wc.hIcon          = LoadIcon(0, IDI_APPLICATION);
        RegisterClassA(&wc);
        _window = CreateWindowA(className, "shared context window", 0, CW_USEDEFAULT, CW_USEDEFAULT, 1, 1, nullptr, 0, wc.hInstance, 0);
        _dc = GetDC(_window);
        if (!SetPixelFormat(_dc, currentPF, &currentPfd)) {
            LGI_LOGE("SetPixelFormat failed!");
            return false;
        }
        _rc = wglCreateContext(_dc);
        if (!_rc) {
            LGI_LOGE("wglCreateContext failed!");
            return false;
        }
        if (!wglShareLists(currentRC, _rc)) {
            LGI_LOGE("wglShareLists failed!");
            return false;
        }
        return true;
    }

    void makeCurrent()
    {
        if (!_rc) {
            LGI_LOGE("shared GL context is not properly initialized.");
            return;
        }

        if (!wglMakeCurrent(_dc, _rc)) {
            LGI_LOGE("wglMakeCurrent() failed.");
        }
    }

private:

    void destroy()
    {
        if (_rc) wglDeleteContext(_rc), _rc = 0;
        if (_dc) ::ReleaseDC(_window, _dc), _dc = 0;
        if (_window) ::DestroyWindow(_window), _window = 0;
    }
};
#elif defined(__ANDROID__) || defined(__linux__)
const char * eglError2String(EGLint err) {
    switch (err) {
    case EGL_SUCCESS:
        return "The last function succeeded without error.";
    case EGL_NOT_INITIALIZED:
        return "EGL is not initialized, or could not be initialized, for the specified EGL display connection.";
    case EGL_BAD_ACCESS:
        return "EGL cannot access a requested resource (for example a context is bound in another thread).";
    case EGL_BAD_ALLOC:
        return "EGL failed to allocate resources for the requested operation.";
    case EGL_BAD_ATTRIBUTE:
        return "An unrecognized attribute or attribute value was passed in the attribute list.";
    case EGL_BAD_CONTEXT:
        return "An EGLContext argument does not name a valid EGL rendering context.";
    case EGL_BAD_CONFIG:
        return "An EGLConfig argument does not name a valid EGL frame buffer configuration.";
    case EGL_BAD_CURRENT_SURFACE:
        return "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.";
    case EGL_BAD_DISPLAY:
        return "An EGLDisplay argument does not name a valid EGL display connection.";
    case EGL_BAD_SURFACE:
        return "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering.";
    case EGL_BAD_MATCH:
        return "Arguments are inconsistent (for example, a valid context requires buffers not supplied by a valid surface).";
    case EGL_BAD_PARAMETER:
        return "One or more argument values are invalid.";
    case EGL_BAD_NATIVE_PIXMAP:
        return "A NativePixmapType argument does not refer to a valid native pixmap.";
    case EGL_BAD_NATIVE_WINDOW:
        return "A NativeWindowType argument does not refer to a valid native window.";
    case EGL_CONTEXT_LOST:
        return "A power management event has occurred. The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering.";
    default:
        return "unknown error";
    }
}
#define EGLCHK_R(x, returnValueWhenFailed)                          \
    if (!(x)) {                                                     \
        LGI_LOGE(#x " failed: %s", eglError2String(eglGetError())); \
        return (returnValueWhenFailed);                             \
    } else                                                          \
        void(0)
#define EGLCHK(x)                                                  \
    if (!(x)) {                                                    \
        LGI_RIP(#x " failed: %s", eglError2String(eglGetError())); \
    } else                                                         \
        void(0)
class RenderContext::Impl {
public:
    Impl(RenderContext::WindowHandle window, bool shared): _window(NativeWindowType(window)) {
        if (shared)
            initSharedContext();
        else
            initStandaloneContext();
    }

    ~Impl() { destroy(); }

    void makeCurrent() {
        if (!eglMakeCurrent(_disp, _surf, _surf, _rc)) { LGI_LOGE("Failed to set current EGL context."); }
    }

    static void clearCurrent() { eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); }

    void swapBuffers() {
        if (!eglSwapBuffers(_disp, _surf)) {
            int error = eglGetError();
            LGI_LOGE("Post record render swap fail. ERROR: %x", error);
        }
    }

private:
    // The context represented by this object.
    bool _new_disp = false;
    EGLDisplay _disp = 0;
    EGLContext _rc = 0;
    EGLSurface _surf = 0;
    NativeWindowType _window = (NativeWindowType) nullptr;

    void initSharedContext() {
        _disp = eglGetCurrentDisplay();
        auto currentRC = eglGetCurrentContext();
        if (!_disp || !currentRC) LGI_RIP("no current display and/or EGL context found.");

        auto currentConfig = getCurrentConfig(_disp, currentRC);
        if (!currentConfig) LGI_RIP("failed to get EGL config.");

        if (_window) {
            LGI_CHK(_surf = eglCreateWindowSurface(_disp, getCurrentConfig(_disp, currentRC), _window, nullptr));
        } else {
            EGLint pbufferAttribs[] = {
                EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE,
            };
            LGI_CHK(_surf = eglCreatePbufferSurface(_disp, currentConfig, pbufferAttribs));
        }

        // create context
        EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION,
            3,
#if LITESPD_GL_ENABLE_DEBUG_BUILD
            EGL_CONTEXT_FLAGS_KHR,
            EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
#endif
            EGL_NONE,
        };
        LGI_CHK(_rc = eglCreateContext(_disp, currentConfig, currentRC, contextAttribs));
    }

    void initStandaloneContext() {
        _new_disp = true;
        _disp = findBestHardwareDisplay();
        if (0 == _disp) { EGLCHK(_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY)); }
        EGLCHK(eglInitialize(_disp, nullptr, nullptr));
        const EGLint configAttribs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 8, EGL_NONE};
        EGLint numConfigs;
        EGLConfig config;
        EGLCHK(eglChooseConfig(_disp, configAttribs, &config, 1, &numConfigs));
        LGI_CHK(numConfigs > 0);
        EGLint pbufferAttribs[] = {
            EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE,
        };
        EGLCHK(_surf = eglCreatePbufferSurface(_disp, config, pbufferAttribs));
        LGI_CHK(_surf);
#ifdef __ANDROID__
        EGLCHK(eglBindAPI(EGL_OPENGL_ES_API));
#else
        EGLCHK(eglBindAPI(EGL_OPENGL_API));
#endif
        EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION,
            3,
#if LITESPD_GL_ENABLE_DEBUG_BUILD
            EGL_CONTEXT_FLAGS_KHR,
            EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
#endif
            EGL_NONE,
        };
        LGI_CHK(_rc = eglCreateContext(_disp, config, 0, contextAttribs));
    }

    void destroy() {
        if (_surf) eglDestroySurface(_disp, _surf), _surf = 0;
        if (_rc) eglDestroyContext(_disp, _rc), _rc = 0;
        if (_new_disp) eglTerminate(_disp), _new_disp = false;
        _disp = 0;
    }

    static EGLConfig getCurrentConfig(EGLDisplay d, EGLContext c) {
        EGLint currentConfigID = 0;
        EGLCHK_R(eglQueryContext(d, c, EGL_CONFIG_ID, &currentConfigID), 0);
        EGLint numConfigs;
        EGLCHK_R(eglGetConfigs(d, nullptr, 0, &numConfigs), 0);
        std::vector<EGLConfig> configs(numConfigs);
        EGLCHK_R(eglGetConfigs(d, configs.data(), numConfigs, &numConfigs), 0);
        for (auto config : configs) {
            EGLint id;
            eglGetConfigAttrib(d, config, EGL_CONFIG_ID, &id);
            if (id == currentConfigID) { return config; }
        }
        LGI_LOGE("Couldn't find current EGL config.");
        return 0;
    }

    // Return the display that represents the best GPU hardware available on current system.
    static EGLDisplay findBestHardwareDisplay() {

        // query required extension
        auto eglQueryDevicesEXT = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(eglGetProcAddress("eglQueryDevicesEXT"));
        auto eglGetPlatformDisplayExt = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
        if (!eglQueryDevicesEXT || !eglGetPlatformDisplayExt) {
            LGI_LOGW("Can't query GPU devices, since required EGL extension(s) are missing. Fallback to default display.");
            return 0;
        }

        EGLDeviceEXT devices[32];
        EGLint num_devices;
        EGLCHK_R(eglQueryDevicesEXT(32, devices, &num_devices), 0);
        if (num_devices == 0) {
            LGI_LOGE("No EGL devices found.");
            return 0;
        }
        LGI_LOGI("Total %d EGL devices found.", num_devices);

        // try find the NVIDIA device
        EGLDisplay nvidia = 0;
        for (int i = 0; i < num_devices; ++i) {
            auto display = eglGetPlatformDisplayExt(EGL_PLATFORM_DEVICE_EXT, devices[i], nullptr);
            EGLint major, minor;
            eglInitialize(display, &major, &minor);
            auto vendor = eglQueryString(display, EGL_VENDOR);
            if (vendor && 0 == strcmp(vendor, "Qualcomm")) nvidia = display;
            eglTerminate(display);
        }

        return nvidia;
    }
};
#else
#include <GLFW/glfw3.h>
class RenderContext::Impl {
public:
    Impl(RenderContext::WindowHandle, bool shared) {
        GLFWwindow * current = nullptr;
        if (shared) {
            current = glfwGetCurrentContext();
            if (!current) {
                LGI_RIP("No current GLFW window found.");
                return;
            }
        } else {
            glfwInit();
        }
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        _window = glfwCreateWindow(1, 1, "", nullptr, current);
        if (!_window) LGI_RIP("Failed to create shared GLFW window.");
    }

    virtual ~Impl() {
        if (_window) glfwDestroyWindow(_window), _window = nullptr;
    }

    static void makeCurrent(RenderContext * ptr) {
        if (_window) {
            glfwMakeContextCurrent(_window);
        } else {
            LGI_LOGE("GL context wasn't properly initialized.");
        }
    }

    static void clearCurrent() { glfwMakeContextCurrent(nullptr); }

    void swapBuffers() { glfwSwapBuffers(_window); }

private:
    GLFWwindow * _window = nullptr;
};
#endif

RenderContext::RenderContext(Type t, WindowHandle w) {
    // store current context
    RenderContextStack rcs;
    rcs.push();

    _impl = new Impl(w, t == SHARED);
    makeCurrent();
    initGLExtensions();

    // switch back to previous context
    rcs.pop();
}
RenderContext::~RenderContext() {
    delete _impl;
    _impl = nullptr;
}
RenderContext & RenderContext::operator=(RenderContext && that) {
    if (this != &that) {
        delete _impl;
        _impl      = that._impl;
        that._impl = nullptr;
    }
    return *this;
}
void RenderContext::makeCurrent() { _impl->makeCurrent(); }
void RenderContext::clearCurrent() { Impl::clearCurrent(); }
void RenderContext::swapBuffers() {
    if (_impl) _impl->swapBuffers();
}

class RenderContextStack::Impl {
    struct OpenGLRC {
#if defined(__ANDROID__) || defined(__linux__)
        EGLDisplay display;
        EGLSurface drawSurface;
        EGLSurface readSurface;
        EGLContext context;

        void store() {
            display     = eglGetCurrentDisplay();
            drawSurface = eglGetCurrentSurface(EGL_DRAW);
            readSurface = eglGetCurrentSurface(EGL_READ);
            context     = eglGetCurrentContext();
        }

        void restore() const {
            if (display && context) {
                if (!eglMakeCurrent(display, drawSurface, readSurface, context)) {
                    EGLint error = eglGetError();
                    LGI_LOGE("Failed to restore EGL context. ERROR: %x", error);
                }
            }
        }
#else
        GLFWwindow * window;

        void store() { window = glfwGetCurrentContext(); }
        void restore() { glfwMakeContextCurrent(window); }
#endif
    };

    std::stack<OpenGLRC> _stack;

public:
    ~Impl() {
        while (_stack.size() > 1) _stack.pop();
        if (1 == _stack.size()) pop();
        LGI_ASSERT(_stack.empty());
    }

    void push() {
        _stack.push({});
        _stack.top().store();
    }

    void apply() {
        if (!_stack.empty()) { _stack.top().restore(); }
    }

    void pop() {
        if (!_stack.empty()) {
            _stack.top().restore();
            _stack.pop();
        }
    }
};
RenderContextStack::RenderContextStack(): _impl(new Impl()) {}
RenderContextStack::~RenderContextStack() { delete _impl; }
void RenderContextStack::push() { _impl->push(); }
void RenderContextStack::apply() { _impl->apply(); }
void RenderContextStack::pop() { _impl->pop(); }

} // namespace LITESPD_GL_NAMESPACE

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
