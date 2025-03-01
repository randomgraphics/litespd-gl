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

/// A monotonically increasing number that uniquely identify the revision of the
/// header.
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

/// \def LITESPD_GL_ENABLE_GLAD
/// Set to 1 to enable GLAD interation helpers. Disabled by default.
#ifndef LITESPD_GL_ENABLE_GLAD
#define LITESPD_GL_ENABLE_GLAD 0
#endif

/// \def LITESPD_GL_THROW
/// The macro to throw runtime exception.
/// \param errorString The error string to throw. Can be std::string or const
/// char*.
#ifndef LITESPD_GL_THROW
#define LITESPD_GL_THROW(message) throw std::runtime_error(message)
#endif

/// \def LITESPD_GL_BACKTRACE
/// Define custom function to retrieve current call stack and store in
/// std::string. This macro is called when litespd-gl encounters critical error,
/// to help quickly identify the source of the error. The default implementation
/// does nothing but return empty string.
#ifndef LITESPD_GL_BACKTRACE
#define LITESPD_GL_BACKTRACE()                                                 \
    std::string("You have to define LITESPD_GL_BACKTRACE to retrieve current " \
                "call stack.")
#endif

/// \def LITESPD_GL_LOG_ERROR
/// The macro to log error message. The default implementation prints to stderr.
/// \paam message The error message to log. The type is const char *.
#ifndef LITESPD_GL_LOG_ERROR
#define LITESPD_GL_LOG_ERROR(message) fprintf(stderr, "[ ERROR ] %s\n", message)
#endif

/// \def LGI_LOGW
/// The macro to log warning message. The default implementation prints to
/// stderr. \param message The warning message to log. The type is const char *.
#ifndef LITESPD_GL_LOG_WARNING
#define LITESPD_GL_LOG_WARNING(message) fprintf(stderr, "[WARNING] %s\n", message)
#endif

/// \def LITESPD_GL_LOG_INFO
/// The macro to log informational message. The default implementation prints to
/// stdout. \param message The message to log. The type is const char *.
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
/// The macro to log debug message. The macro is ignored when
/// LITESPD_GL_ENABLE_DEBUG_BUILD is 0 \param message The message to log. The
/// type is const char *.
#ifndef LITESPD_GL_LOG_DEBUG
#define LITESPD_GL_LOG_DEBUG(message) fprintf(stdout, "[ DEBUG ] %s\n", message)
#endif

/// \def LITESPD_GL_ASSERT
/// The runtime assert macro for debug build only. This macro has no effect when
/// LITESPD_GL_ENABLE_DEBUG_BUILD is 0.
#ifndef LITESPD_GL_ASSERT
#define LITESPD_GL_ASSERT(expression, ...)                                         \
    if (!(expression)) {                                                           \
        auto errorMessage__ = LITESPD_GL_NAMESPACE::format(__VA_ARGS__);           \
        LGI_LOGE("Condition " #expression " not met. %s", errorMessage__.c_str()); \
        assert(false);                                                             \
    } else                                                                         \
        void(0)
#endif

// ---------------------------------------------------------------------------------------------------------------------
// include headers

#if LITESPD_GL_ENABLE_GLAD
#include <glad/glad.h>
#else
/// Make sure GL/gl.h is included before this header.
#ifndef GL_VERSION_1_0
#error "GL/gl.h must be included before including litespd-gl.h"
#endif
#endif

#if LITESPD_GL_ENABLE_GLFW3
#ifndef _glfw3_h_
#include <GLFW/glfw3.h>
#endif
#endif

#include <cassert>

// ---------------------------------------------------------------------------------------------------------------------
// Define LGI macros. LGI stands for Litespd-GL-Implementation. Macros started
// with this prefix are reserved for Litespd-GL internal use.

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

#define LGI_THROW(...)                                                                                     \
    do {                                                                                                   \
        std::stringstream errorStream_;                                                                    \
        errorStream_ << __FILE__ << "(" << __LINE__ << "): " << LITESPD_GL_NAMESPACE::format(__VA_ARGS__); \
        auto errorString_ = errorStream_.str();                                                            \
        LGI_LOGE("%s", errorString_.data());                                                               \
        LITESPD_GL_THROW(errorString_);                                                                    \
    } while (false)

#define LGI_REQUIRE(condition, ...)                                                    \
    do {                                                                               \
        if (!(condition)) {                                                            \
            auto errorMessage__ = LITESPD_GL_NAMESPACE::format(__VA_ARGS__);           \
            LGI_THROW("Condition " #condition " not met. %s", errorMessage__.c_str()); \
        }                                                                              \
    } while (false)

// Check OpenGL error. This check is enabled in both debug and release build.
#define LGI_CHK(func)                                                                             \
    if (true) {                                                                                   \
        func;                                                                                     \
        if (glGetError == NULL) {                                                                 \
            LGI_LOGE("gl not initialized properly...");                                          \
        } else {                                                                                  \
            GLenum err = glGetError();                                                            \
            if (GL_NO_ERROR != err) { LGI_LOGE("function %s failed. (error=0x%x)", #func, err); } \
        }                                                                                         \
    } else                                                                                        \
        void(0)

// Use LGI_DCHK() at where that you want to have some sanity check only in
// debug build,
#if LITESPD_GL_ENABLE_DEBUG_BUILD
#define LGI_DCHK(x) LGI_CHK(x)
#else
#define LGI_DCHK(x) x
#endif

#if LITESPD_GL_ENABLE_DEBUG_BUILD
// assert is enabled only in debug build
#define LGI_ASSERT LITESPD_REQUIRE
#else
#define LGI_ASSERT(...) ((void) 0)
#endif

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

#if LITESPD_GL_ENABLE_GLAD
/// @brief Load all GL extension functions using GLAD.
void initGlad(bool printGLInfo = false);
#endif

// The 'optionalFilename' parameter is optional and is only used when printing
// shader compilation error.
GLuint loadShaderFromString(const char * source, size_t length, GLenum shaderType, const char * optionalFilename = nullptr);

// the program name parameter is optional and is only used to print link error.
GLuint linkProgram(const std::vector<GLuint> & shaders, const char * optionalProgramName = nullptr);

// a utility function to upload uniform values
template<typename T>
void updateUniformValue(GLint location, const T & value) {
    if (location < 0) return;

    if constexpr (std::is_same<int, T>()) {
        LGI_DCHK(glUniform1i(location, (int) value));
    } else if constexpr (std::is_same<unsigned int, T>()) {
        LGI_DCHK(glUniform1ui(location, (unsigned int) value));
    } else if constexpr (std::is_same<float, T>()) {
        LGI_DCHK(glUniform1f(location, value));
    } else if constexpr (std::is_same<glm::vec2, T>()) {
        LGI_DCHK(glUniform2fv(location, 1, (const float *) &value));
    } else if constexpr (std::is_same<glm::vec3, T>()) {
        LGI_DCHK(glUniform3fv(location, 1, (const float *) &value));
    } else if constexpr (std::is_same<glm::vec4, T>()) {
        LGI_DCHK(glUniform4fv(location, 1, (const float *) &value));
    } else if constexpr (std::is_same<glm::mat3, T>()) {
        LGI_DCHK(glUniformMatrix3fv(location, 1, false, (const float *) &value));
    } else if constexpr (std::is_same<glm::mat4, T>()) {
        LGI_DCHK(glUniformMatrix4fv(location, 1, false, (const float *) &value));
    } else if constexpr (std::is_same<std::vector<float>, T>()) {
        auto count = static_cast<GLsizei>(value.size());
        LGI_DCHK(glUniform1fv(location, count, value.data()));
    } else {
        struct DependentFalse : public std::false_type {};
        static_assert(DependentFalse::value, "unsupported uniform type");
    }
}

inline void clearScreen(const glm::vec4 & color = {0.f, 0.f, 0.f, 1.f}, float depth = 1.0f, uint32_t stencil = 0,
                        GLbitfield flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) {
    if (flags | GL_COLOR_BUFFER_BIT) glClearColor(color.x, color.y, color.z, color.w);
    if (flags | GL_DEPTH_BUFFER_BIT) glClearDepthf(depth);
    if (flags | GL_STENCIL_BUFFER_BIT) glClearStencil(stencil);
    LGI_DCHK(glClear(flags));
}

// -----------------------------------------------------------------------------
//
inline GLint getInt(GLenum name) {
    GLint value;
    glGetIntegerv(name, &value);
    return value;
}

inline GLint getInt(GLenum name, GLint i) {
    GLint value;
    glGetIntegeri_v(name, i, &value);
    return value;
}

#if defined(__ANDROID__) || defined(__linux__)
const char * eglError2String(EGLint err);
#endif

template<GLenum TARGET>
struct QueryObject {
    enum Status {
        EMPTY,   // the query object is not created yet.
        IDLE,    // the query object is idle and ready to use.
        RUNNING, // in between begin() and end()
        PENDING, // query is issued. but result is yet to returned.
    };

    GLuint qo     = 0;
    Status status = EMPTY;

    QueryObject() = default;
    ~QueryObject() { cleanup(); }

    LGI_NO_COPY(QueryObject);

    // can move
    QueryObject(QueryObject && that) {
        qo          = that.qo;
        status      = that.status;
        that.qo     = 0;
        that.status = EMPTY;
    }
    QueryObject & operator=(QueryObject && that) {
        if (this != &that) {
            qo          = that.qo;
            status      = that.status;
            that.qo     = 0;
            that.status = EMPTY;
        }
    }

    bool empty() const { return EMPTY == status; }
    bool idle() const { return IDLE == status; }
    bool running() const { return RUNNING == status; }
    bool pending() const { return PENDING == status; }

    void cleanup() {
#ifdef __ANDROID__
        if (qo) glDeleteQueriesEXT(1, &qo), qo = 0;
#else
        if (qo) glDeleteQueries(1, &qo), qo = 0;
#endif
        status = IDLE;
    }

    void allocate() {
        cleanup();
#ifdef __ANDROID__
        LGI_DCHK(glGenQueriesEXT(1, &qo));
#else
        LGI_DCHK(glGenQueries(1, &qo));
#endif
        status = IDLE;
    }

    void begin() {
        if (IDLE == status) {
#ifdef __ANDROID__
            LGI_DCHK(glBeginQueryEXT(TARGET, qo));
#else
            LGI_DCHK(glBeginQuery(TARGET, qo));
#endif
            status = RUNNING;
        }
    }

    void end() {
        if (RUNNING == status) {
#ifdef __ANDROID__
            LGI_DCHK(glEndQueryEXT(TARGET));
#else
            LGI_DCHK(glEndQuery(TARGET));
#endif
            status = PENDING;
        }
    }

    void mark() {
        if (IDLE == status) {
#ifdef __ANDROID__
            glQueryCounterEXT(qo, TARGET);
#else
            glQueryCounter(qo, TARGET);
#endif
            status = PENDING;
        }
    }

    bool getResult(uint64_t & result) {
        if (PENDING != status) return false;
        GLint available;
#ifdef __ANDROID__
        glGetQueryObjectivEXT(qo, GL_QUERY_RESULT_AVAILABLE, &available);
#else
        glGetQueryObjectiv(qo, GL_QUERY_RESULT_AVAILABLE, &available);
#endif
        if (!available) return false;

#ifdef __ANDROID__
        LGI_DCHK(glGetQueryObjectui64vEXT(qo, GL_QUERY_RESULT, &result));
#else
        LGI_DCHK(glGetQueryObjectui64v(qo, GL_QUERY_RESULT, &result));
#endif
        status = IDLE;
        return true;
    }

    // returns 0, if the query is still pending.
    template<uint64_t DEFAULT_VALUE = 0>
    uint64_t getResult() const {
        uint64_t ret = DEFAULT_VALUE;
        return getResult(ret) ? ret : DEFAULT_VALUE;
    }
};

// -----------------------------------------------------------------------------
// Helper class to manage GL buffer object.
template<GLenum TARGET, size_t MIN_GPU_BUFFER_LENGH = 0>
struct BufferObject {
    GLuint bo            = 0;
    size_t length        = 0; // buffer length in bytes.
    GLenum mapped_target = 0;

    LGI_NO_COPY(BufferObject);
    LGI_NO_MOVE(BufferObject);

    BufferObject() {}

    ~BufferObject() { cleanup(); }

    static GLenum GetTarget() { return TARGET; }

    template<typename T, GLenum T2 = TARGET>
    void allocate(size_t count, const T * ptr, GLenum usage = GL_STATIC_DRAW) {
        cleanup();
        LGI_CHK(glGenBuffers(1, &bo));
        // Note: ARM Mali GPU doesn't work well with zero sized buffers. So
        // we create buffer that is large enough to hold at least one element.
        length = std::max(count, MIN_GPU_BUFFER_LENGH) * sizeof(T);
        LGI_CHK(glBindBuffer(T2, bo));
        LGI_CHK(glBufferData(T2, length, ptr, usage));
        LGI_CHK(glBindBuffer(T2, 0)); // unbind
    }

    void cleanup() {
        if (bo) glDeleteBuffers(1, &bo), bo = 0;
        length = 0;
    }

    bool empty() const { return 0 == bo; }

    template<typename T, GLenum T2 = TARGET>
    void update(const T * ptr, size_t offset = 0, size_t count = 1) {
        LGI_DCHK(glBindBuffer(T2, bo));
        LGI_DCHK(glBufferSubData(T2, offset * sizeof(T), count * sizeof(T), ptr));
    }

    template<GLenum T2 = TARGET>
    void bind() const {
        LGI_DCHK(glBindBuffer(T2, bo));
    }

    template<GLenum T2 = TARGET>
    static void unbind() {
        LGI_DCHK(glBindBuffer(T2, 0));
    }

    template<GLenum T2 = TARGET>
    void bindBase(GLuint base) const {
        LGI_DCHK(glBindBufferBase(T2, base, bo));
    }

    template<typename T, GLenum T2 = TARGET>
    void getData(T * ptr, size_t offset, size_t count) {
        LGI_DCHK(glBindBuffer(T2, bo));
        void * mapped = nullptr;
        LGI_DCHK(mapped = glMapBufferRange(T2, offset * sizeof(T), count * sizeof(T), GL_MAP_READ_BIT));
        if (mapped) {
            memcpy(ptr, mapped, count * sizeof(T));
            LGI_DCHK(glUnmapBuffer(T2));
        }
    }

    template<GLenum T2 = TARGET>
    void * map(size_t offset, size_t count) {
        bind();
        void * ptr = nullptr;
        LGI_DCHK(ptr = glMapBufferRange(T2, offset, count, GL_MAP_READ_BIT));
        assert(ptr);
        mapped_target = TARGET;
        return ptr;
    }

    template<GLenum T2 = TARGET>
    void * map() {
        return map<T2>(0, length);
    }

    void unmap() {
        if (mapped_target) {
            bind();
            LGI_DCHK(glUnmapBuffer(mapped_target));
            mapped_target = 0;
        }
    }

    operator GLuint() const { return bo; }
};

// -----------------------------------------------------------------------------
//
template<typename T, GLenum TARGET, size_t MIN_GPU_BUFFER_LENGTH = 0>
struct TypedBufferObject {
    std::vector<T>                                  c; // CPU data
    gl::BufferObject<TARGET, MIN_GPU_BUFFER_LENGTH> g; // GPU data

    void allocateGpuBuffer() { g.allocate(c.size(), c.data()); }

    void syncGpuBuffer() { g.update(c.data(), 0, c.size()); }

    // Synchornosly copy buffer content from GPU to CPU.
    // Note that this call is EXTREMELY expensive, since it stalls both CPU and
    // GPU.
    void syncToCpu() {
        glFinish();
        g.getData(c.data(), 0, c.size());
    }

    void cleanup() {
        c.clear();
        g.cleanup();
    }
};

// -----------------------------------------------------------------------------
//
template<typename T, GLenum TARGET1, GLenum TARGET2, size_t MIN_GPU_BUFFER_LENGTH = 0>
struct TypedBufferObject2 {
    std::vector<T>                                   c;  // CPU data
    gl::BufferObject<TARGET1, MIN_GPU_BUFFER_LENGTH> g1; // GPU data
    gl::BufferObject<TARGET2, MIN_GPU_BUFFER_LENGTH> g2; // GPU data

    void allocateGpuBuffer() {
        g1.allocate(c.size(), c.data());
        g2.allocate(c.size(), c.data());
    }

    void syncGpuBuffer() {
        g1.update(c.data(), 0, c.size());
        g2.update(c.data(), 0, c.size());
    }

    void cleanup() {
        c.clear();
        g1.cleanup();
        g2.cleanup();
    }

    template<GLenum TT>
    void bind() const {
        if constexpr (TT == TARGET1) {
            g1.bind();
        } else {
            static_assert(TT == TARGET2);
            g2.bind();
        }
    }

    template<GLenum TT>
    void bindBase(GLuint base) const {
        if constexpr (TT == TARGET1) {
            g1.bindBase(base);
        } else {
            static_assert(TT == TARGET2);
            g2.bindBase(base);
        }
    }
};

// -----------------------------------------------------------------------------
//
class VertexArrayObject {
    GLuint _va = 0;

public:
    ~VertexArrayObject() { cleanup(); }

    void allocate() {
        cleanup();
        LGI_CHK(glGenVertexArrays(1, &_va));
    }

    void cleanup() {
        if (_va) glDeleteVertexArrays(1, &_va), _va = 0;
    }

    void bind() const { LGI_DCHK(glBindVertexArray(_va)); }

    void unbind() const { LGI_DCHK(glBindVertexArray(0)); }

    operator GLuint() const { return _va; }
};

// -----------------------------------------------------------------------------
//
struct AutoShader {
    GLuint shader;

    AutoShader(GLuint s = 0): shader(s) {}
    ~AutoShader() { cleanup(); }

    void cleanup() {
        if (shader) glDeleteShader(shader), shader = 0;
    }

    LGI_NO_COPY(AutoShader);

    // can move
    AutoShader(AutoShader && rhs): shader(rhs.shader) { rhs.shader = 0; }
    AutoShader & operator=(AutoShader && rhs) {
        if (this != &rhs) {
            cleanup();
            shader     = rhs.shader;
            rhs.shader = 0;
        }
        return *this;
    }

    operator GLuint() const { return shader; }
};

class SamplerObject {
    GLuint _id = 0;

public:
    SamplerObject() {}
    ~SamplerObject() { cleanup(); }

    LGI_NO_COPY(SamplerObject);

    // can move
    SamplerObject(SamplerObject && that) {
        _id      = that._id;
        that._id = 0;
    }
    SamplerObject & operator=(SamplerObject && that) {
        if (this != &that) {
            cleanup();
            _id      = that._id;
            that._id = 0;
        }
        return *this;
    }

    operator GLuint() const { return _id; }

    void allocate() {
        cleanup();
        glGenSamplers(1, &_id);
    }
    void cleanup() {
        if (_id) glDeleteSamplers(1, &_id), _id = 0;
    }
    void bind(size_t unit) const {
        LGI_ASSERT(glIsSampler(_id));
        glBindSampler((GLuint) unit, _id);
    }
};

inline void bindTexture(GLenum target, uint32_t stage, GLuint texture) {
    LGI_DCHK(glActiveTexture(GL_TEXTURE0 + stage));
    LGI_DCHK(glBindTexture(target, texture));
}

class TextureObject {
public:
    // no copy
    TextureObject(const TextureObject &)             = delete;
    TextureObject & operator=(const TextureObject &) = delete;

    // can move
    TextureObject(TextureObject && rhs) noexcept: _desc(rhs._desc) { rhs._desc.id = 0; }
    TextureObject & operator=(TextureObject && rhs) noexcept {
        if (this != &rhs) {
            cleanup();
            _desc        = rhs._desc;
            rhs._desc.id = 0;
        }
        return *this;
    }

    // default constructor
    TextureObject() { cleanup(); }

    ~TextureObject() { cleanup(); }

    struct TextureDesc {
        GLuint   id = 0; // all other fields are undefined, if id is 0.
        GLenum   target;
        GLint    internalFormat;
        uint32_t width;
        uint32_t height;
        uint32_t depth; // this is number of layers for 2D array texture and is
                        // always 6 for cube texture.
        uint32_t mips;
    };

    const TextureDesc & getDesc() const { return _desc; }

    GLenum target() const { return _desc.target; }
    GLenum id() const { return _desc.id; }

    bool empty() const { return 0 == _desc.id; }

    bool is2D() const { return GL_TEXTURE_2D == _desc.target; }

    bool isArray() const { return GL_TEXTURE_2D_ARRAY == _desc.target; }

    void attach(GLenum target, GLuint id);

    void attach(const TextureObject & that) { attach(that._desc.target, that._desc.id); }

    void allocate2D(jedi::ColorFormat f, size_t w, size_t h, size_t m = 1);

    void allocate2DArray(jedi::ColorFormat f, size_t w, size_t h, size_t l, size_t m = 1);

    void allocateCube(jedi::ColorFormat f, size_t w, size_t m = 1);

    void setPixels(size_t level, size_t x, size_t y, size_t w, size_t h,
                   size_t       rowPitchInBytes, // set to 0, if pixels are tightly packed.
                   const void * pixels) const;

    // Set to rowPitchInBytes 0, if pixels are tightly packed.
    void setPixels(size_t layer, size_t level, size_t x, size_t y, size_t w, size_t h, size_t rowPitchInBytes, const void * pixels) const;

    jedi::ManagedRawImage getBaseLevelPixels() const;

    void cleanup() {
        if (_owned && _desc.id) { LGI_CHK(glDeleteTextures(1, &_desc.id)); }
        _desc.id             = 0;
        _desc.target         = GL_NONE;
        _desc.internalFormat = GL_NONE;
        _desc.width          = 0;
        _desc.height         = 0;
        _desc.depth          = 0;
        _desc.mips           = 0;
    }

    void bind(size_t stage) const {
        LGI_DCHK(glActiveTexture(GL_TEXTURE0 + (int) stage));
        LGI_DCHK(glBindTexture(_desc.target, _desc.id));
    }

    void unbind() const { glBindTexture(_desc.target, 0); }

    operator GLuint() const { return _desc.id; }

private:
    TextureDesc _desc;
    bool        _owned = false;
    void        applyDefaultParameters();
};

// -----------------------------------------------------------------------------
// Helper class to manage frame buffer object.
class SimpleFBO {
    struct MipLevel {
        uint32_t width = 0, height = 0;
        uint32_t fbo = 0;
    };

    struct RenderTarget {
        GLint  internalFormat = 0;
        GLuint texture = 0;
    };

    GLenum                _colorTextureTarget = GL_TEXTURE_2D;
    std::vector<MipLevel> _mips;
    RenderTarget          _colors[8] = {};
    GLuint                _depth     = 0;
    const GLsizei         COLOR_BUFFER_COUNT;
    const bool            HAS_DEPTH;

public:
    SimpleFBO(GLsizei colorBufferCount = 1, bool depth = true): COLOR_BUFFER_COUNT(colorBufferCount), HAS_DEPTH(depth) {
        LGI_ASSERT(std::size_t(colorBufferCount) < std::size(_colors));
    }

    ~SimpleFBO() { cleanup(); }

    void cleanup();

    void allocate(uint32_t w, uint32_t h, uint32_t levels, const GLint * cf);

    void allocate(uint32_t w, uint32_t h, uint32_t levels, GLint cf) {
        LGI_CHK(1 == COLOR_BUFFER_COUNT);
        allocate(w, h, levels, &cf);
    }

    void allocate(uint32_t w, uint32_t h, uint32_t levels, GLint cf1, GLint cf2) {
        LGI_CHK(2 == COLOR_BUFFER_COUNT);
        jedi::ColorFormat formats[] = {cf1, cf2};
        allocate(w, h, levels, formats);
    }

    uint32_t getLevels() const { return (uint32_t) _mips.size(); }

    uint32_t getWidth(uint32_t level) const { return _mips[level].width; }

    uint32_t getHeight(uint32_t level) const { return _mips[level].height; }

    uint32_t getFBO(size_t level) const { return _mips[level].fbo; }

    void setColorTextureFilter(uint32_t rt, GLenum minFilter, GLenum maxFilter) {
        LGI_DCHK(glBindTexture(_colorTextureTarget, _colors[rt].texture));
        LGI_DCHK(glTexParameteri(_colorTextureTarget, GL_TEXTURE_MIN_FILTER, minFilter));
        LGI_DCHK(glTexParameteri(_colorTextureTarget, GL_TEXTURE_MAG_FILTER, maxFilter));
    }

    void bind(size_t level = 0) const {
        for (GLsizei i = 0; i < COLOR_BUFFER_COUNT + 1; ++i) {
            glActiveTexture(GLenum(GL_TEXTURE0 + i));
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        const auto & m = _mips[level];
        glBindFramebuffer(GL_FRAMEBUFFER, m.fbo);
        glViewport(0, 0, (GLsizei) m.width, (GLsizei) m.height);
    }

    void bindColorAsTexture(uint32_t rt, uint32_t stage) const {
        LGI_ASSERT(rt < (uint32_t) COLOR_BUFFER_COUNT);
        glActiveTexture(GLenum(GL_TEXTURE0 + stage));
        glBindTexture(_colorTextureTarget, _colors[rt].texture);
    }

    void bindDepthAsTexture(uint32_t stage) const {
        glActiveTexture(GLenum(GL_TEXTURE0 + stage));
        glBindTexture(GL_TEXTURE_2D, _depth);
    }

    GLenum getColorTarget() const { return _colorTextureTarget; }
    GLuint getColorTexture(size_t rt) const { return _colors[rt].texture; }
    GLint  getColorFormat(size_t rt) const { return _colors[rt].internalFormat; }

    void saveColorToFile(uint32_t rt, const std::string & filepath) const;
    void saveDepthToFile(const std::string & filepath) const;
};

// -----------------------------------------------------------------------------
// Helper class to manage cube map frame buffer object
class CubeFBO {
    struct MipLevel {
        uint32_t width;
        uint32_t fbo[6];
    };

    std::vector<MipLevel> _mips;
    GLuint                _color = 0, _depth = 0;

public:
    ~CubeFBO() { cleanup(); }

    void cleanup() {
        if (_color) glDeleteTextures(1, &_color), _color = 0;
        if (_depth) glDeleteTextures(1, &_depth), _depth = 0;
        for (size_t i = 0; i < _mips.size(); ++i) {
            for (int f = 0; f < 6; ++f) {
                if (_mips[i].fbo[f]) glDeleteFramebuffers(1, &_mips[i].fbo[f]), _mips[i].fbo[f] = 0;
            }
        }
        _mips.clear();
    }

    void allocate(uint32_t w, uint32_t levels, jedi::ColorFormat cf);

    uint32_t getLevels() const { return (uint32_t) _mips.size(); }

    uint32_t getWidth(size_t level) const { return _mips[level].width; }

    GLuint getColorTexture() const { return _color; }

    GLuint getDepthTexture() const { return _depth; }

    void bind(uint32_t face, uint32_t level = 0) const {
        const auto & m = _mips[level];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, m.fbo[face]);
        glViewport(0, 0, (GLsizei) m.width, (GLsizei) m.width);
    }

    void bindColorAsTexture(int slot = -1) const {
        GLenum stage = (slot >= 0) ? slot : 0;
        glActiveTexture(GLenum(GL_TEXTURE0 + stage));
        glBindTexture(GL_TEXTURE_CUBE_MAP, _color);
    }

    void bindDepthAsTexture(int slot = -1) const {
        GLenum stage = (slot >= 0) ? slot : 1;
        glActiveTexture(GLenum(GL_TEXTURE0 + stage));
        glBindTexture(GL_TEXTURE_CUBE_MAP, _depth);
    }
};

// SSBO for in-shader debug output. Check out ftl/main_ps.glsl for example
// usage. It is currently working on Windows only. Running it on Android crashes
// the driver.
struct DebugSSBO {
#if LITESPD_GL_ENABLE_DEBUG_BUILD
    std::vector<float>                         buffer;
    mutable std::vector<float>                 printed;
    int *                                      counter = nullptr;
    gl::BufferObject<GL_SHADER_STORAGE_BUFFER> g;
#endif

    static constexpr bool isEnabled() {
#if LITESPD_GL_ENABLE_DEBUG_BUILD
        return true;
#else
        return false;
#endif
    }

    ~DebugSSBO() { cleanup(); }

    void allocate(size_t n) {
#if LITESPD_GL_ENABLE_DEBUG_BUILD
        cleanup();
        buffer.resize(n + 1);
        printed.resize(buffer.size());
        counter = (int *) &buffer[0];
        g.allocate(buffer.size(), buffer.data(), GL_STATIC_READ);
#else
        (void) n;
#endif
    }

    void bind(int slot = 15) const {
#if LITESPD_GL_ENABLE_DEBUG_BUILD
        if (g) glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, g);
#else
        (void) slot;
#endif
    }

    void cleanup() {
#if LITESPD_GL_ENABLE_DEBUG_BUILD
        buffer.clear();
        printed.clear();
        counter = nullptr;
        g.cleanup();
#endif
    }

    void clearCounter() {
#if LITESPD_GL_ENABLE_DEBUG_BUILD
        if (!counter) return;
        *counter = 0;
        g.update(counter, 0, 1);
#endif
    }

    void pullDataFromGPU() {
#if LITESPD_GL_ENABLE_DEBUG_BUILD
        if (buffer.empty()) return;
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        g.getData(buffer.data(), 0, buffer.size());
#endif
    }

    void printLastResult() const;
};

struct ScreenQuad {
    struct Vertex {
        float x, y;
        float u, v;
    };

    // vertex array
    GLuint                            va = 0;
    gl::BufferObject<GL_ARRAY_BUFFER> vb;

    ScreenQuad() {}

    ~ScreenQuad() { cleanup(); }

    static glm::vec4 fullTexture() { return glm::vec4(0.f, 1.f, 1.f, 0.f); };

    static glm::vec4 fullScreen() { return glm::vec4(-1.f, 1.f, 1.f, -1.f); }

    ScreenQuad & allocate();

    ScreenQuad & cleanup();

    /// specify 2D sprite to screen.
    /// Use glm::vec4 to specify the rectangle:
    ///     x: left
    ///     y: right
    ///     z: top
    ///     w: bottom
    ScreenQuad & update(const glm::vec4 pos = fullScreen(), const glm::vec4 & uv = fullTexture());

    const ScreenQuad & draw() const {
        LGI_ASSERT(va);
        LGI_DCHK(glBindVertexArray(va));
        LGI_DCHK(glDrawArrays(GL_TRIANGLES, 0, 6));
        return *this;
    }
};

class SimpleGlslProgram {
    GLuint _program = 0;

    // struct Uniform
    //{
    //     GLint location;
    // };
    // std::vector<Uniform> _uniforms;

public:
    // optional program name (for debug log)
    std::string name;

#ifdef _DEBUG
    std::string vsSource, psSource, csSource;
#endif

    LGI_NO_COPY(SimpleGlslProgram);
    LGI_NO_MOVE(SimpleGlslProgram);

    SimpleGlslProgram(const char * optionalProgramName = nullptr) {
        if (optionalProgramName) name = optionalProgramName;
    }

    ~SimpleGlslProgram() { cleanup(); }

    bool loadVsPs(const char * vscode, const char * pscode) {
#ifdef _DEBUG
        if (vscode) vsSource = vscode;
        if (pscode) psSource = pscode;
#endif
        cleanup();
        AutoShader vs = loadShaderFromString(vscode, 0, GL_VERTEX_SHADER, name.c_str());
        AutoShader ps = loadShaderFromString(pscode, 0, GL_FRAGMENT_SHADER, name.c_str());
        if ((vscode && !vs) || (pscode && !ps)) return false;
        _program = linkProgram({vs, ps}, name.c_str());
        return _program != 0;
    }

    bool loadCs(const char * code) {
#ifdef _DEBUG
        if (code) csSource = code;
#endif
        cleanup();
        AutoShader cs = loadShaderFromString(code, 0, GL_COMPUTE_SHADER, name.c_str());
        if (!cs) return false;
        _program = linkProgram({cs}, name.c_str());

        return _program != 0;
    }

    // void InitUniform(const char* name)
    //{
    //     auto loc = glGetUniformLocation(_program, name);
    //     if (-1 != loc) {
    //         _uniforms.push_back({ loc });
    //     }
    // }

    // template<typename T>
    // void UpdateUniform(size_t index, const T& value) const
    //{
    //     gl::UpdateUniformValue(_uniforms[index].location, value);
    // }

    void use() const { LGI_DCHK(glUseProgram(_program)); }

    void cleanup() {
        //_uniforms.clear();
        if (_program) glDeleteProgram(_program), _program = 0;
    }

    GLint getUniformLocation(const char * name_) const { return glGetUniformLocation(_program, name_); }

    GLint getUniformBinding(const char * name_) const {
        auto loc = glGetUniformLocation(_program, name_);
        if (-1 == loc) return -1;
        GLint binding;
        glGetUniformiv(_program, loc, &binding);
        return binding;
    }

    operator GLuint() const { return _program; }
};

class SimpleUniform {
public:
    using Value = std::variant<int, unsigned int, float, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::uvec2, glm::uvec3,
                               glm::uvec4, glm::mat3x3, glm::mat4x4, std::vector<float>>;

    Value value;

    SimpleUniform(std::string name): _name(name) {}

    template<typename T>
    SimpleUniform(std::string name, const T & v): value(v), _name(name) {}

    bool init(GLuint program) {
        if (program > 0) {
            LGI_DCHK(_location = glGetUniformLocation(program, _name.c_str()));
        } else {
            _location = -1;
        }
        return _location > -1;
    }

    void apply() const {
        if (_location < 0) return;
        std::visit(
            [&](auto && v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, int>)
                    glUniform1i(_location, v);
                else if constexpr (std::is_same_v<T, unsigned int>)
                    glUniform1ui(_location, v);
                else if constexpr (std::is_same_v<T, float>)
                    glUniform1f(_location, v);
                else if constexpr (std::is_same_v<T, glm::vec2>)
                    glUniform2f(_location, v.x, v.y);
                else if constexpr (std::is_same_v<T, glm::vec3>)
                    glUniform3f(_location, v.x, v.y, v.z);
                else if constexpr (std::is_same_v<T, glm::vec4>)
                    glUniform4f(_location, v.x, v.y, v.z, v.w);
                else if constexpr (std::is_same_v<T, glm::ivec2>)
                    glUniform2i(_location, v.x, v.y);
                else if constexpr (std::is_same_v<T, glm::ivec3>)
                    glUniform3i(_location, v.x, v.y, v.z);
                else if constexpr (std::is_same_v<T, glm::ivec4>)
                    glUniform4i(_location, v.x, v.y, v.z, v.w);
                else if constexpr (std::is_same_v<T, glm::uvec2>)
                    glUniform2ui(_location, v.x, v.y);
                else if constexpr (std::is_same_v<T, glm::uvec3>)
                    glUniform3ui(_location, v.x, v.y, v.z);
                else if constexpr (std::is_same_v<T, glm::uvec4>)
                    glUniform4ui(_location, v.x, v.y, v.z, v.w);
                else if constexpr (std::is_same_v<T, glm::mat3x3>)
                    glUniformMatrix3fv(_location, 1, false, glm::value_ptr(v));
                else if constexpr (std::is_same_v<T, glm::mat4x4>)
                    glUniformMatrix4fv(_location, 1, false, glm::value_ptr(v));
                else if constexpr (std::is_same_v<T, const std::vector<float>>)
                    glUniform1fv(_location, (GLsizei) v.size(), v.data());
            },
            value);
    }

private:
    const std::string _name;
    GLint             _location = -1;
};

class SimpleSprite {
    SimpleGlslProgram _program;
    GLint             _tex0Binding = -1;
    ScreenQuad        _quad;
    GLuint            _sampler = 0;

public:
    LGI_NO_COPY(SimpleSprite);
    LGI_NO_MOVE(SimpleSprite);

    SimpleSprite() = default;
    ~SimpleSprite() { cleanup(); }
    bool init();
    void cleanup();
    void draw(GLuint texture, const glm::vec4 & pos = ScreenQuad::fullScreen(), const glm::vec4 & uv = ScreenQuad::fullTexture());
};

class SimpleTextureCopy {
    struct CopyProgram {
        SimpleGlslProgram program;
        GLint             tex0Binding = -1;
    };
    std::unordered_map<GLuint, CopyProgram> _programs; // key is texture target.
    ScreenQuad                              _quad;
    GLuint                                  _sampler = 0;
    GLuint                                  _fbo     = 0;

public:
    LGI_NO_COPY(SimpleTextureCopy);
    LGI_NO_MOVE(SimpleTextureCopy);
    SimpleTextureCopy() {}
    ~SimpleTextureCopy() { cleanup(); }
    bool init();
    void cleanup();
    struct TextureSubResource {
        GLenum   target;
        GLuint   id;
        uint32_t level;
        uint32_t z; // layer index for 2d array texture.
                    // TODO: uint32_t x, y, w, h;
    };
    void copy(const TextureSubResource & src, const TextureSubResource & dst);
    void copy(const TextureObject & src, uint32_t srcLevel, uint32_t srcZ, const TextureObject & dst, uint32_t dstLevel, uint32_t dstZ) {
        auto & s = src.getDesc();
        auto & d = dst.getDesc();
        copy({s.target, s.id, srcLevel, srcZ}, {d.target, d.id, dstLevel, dstZ});
    }
};

// -----------------------------------------------------------------------------
// For asynchronous timer (not time stamp) queries
struct GpuTimeElapsedQuery {

    std::string name;

    explicit GpuTimeElapsedQuery(std::string n = std::string("")): name(n) { _q.allocate(); }

    ~GpuTimeElapsedQuery() {}

    // returns duration in nanoseconds
    GLuint64 duration() const { return _result; }

    void start() { _q.begin(); }

    void stop();

    // Print stats to string
    std::string print() const;

    friend inline std::ostream & operator<<(std::ostream & s, const GpuTimeElapsedQuery & t) {
        s << t.print();
        return s;
    }

private:
    QueryObject<GL_TIME_ELAPSED> _q;
    uint64_t                     _result = 0;
};

// -----------------------------------------------------------------------------
// GPU time stamp query
class GpuTimestamps {
public:
    explicit GpuTimestamps(std::string name = std::string("")): _name(name) {}

    void start() {
        LGI_ASSERT(!_started);
        if (!_started) {
            _started = true;
            _count   = 0;
            mark("start time");
        }
    }

    void stop() {
        LGI_ASSERT(_started);
        if (_started) {
            mark("end time");
            _started = false;
        }
    }

    void mark(std::string name) {
        LGI_ASSERT(_started);
        if (!_started) return;

        if (_count == _marks.size()) {
            _marks.emplace_back();
            _marks.back().name = name;
        }

        LGI_ASSERT(_count < _marks.size());
        _marks[_count++].mark();
    }

    // Print stats of timestamps to string
    std::string print(const char * ident = nullptr) const;

private:
    struct Query {
        std::string               name;
        QueryObject<GL_TIMESTAMP> q;
        uint64_t                  result = 0;
        Query() { q.allocate(); }
        LGI_NO_COPY(Query);
        LGI_DEFAULT_MOVE(Query);
        void mark() {
            if (q.idle()) {
                q.mark();
            } else {
                q.getResult(result);
            }
        }
    };

    std::string        _name;
    std::vector<Query> _marks;
    size_t             _count   = 0;
    bool               _started = false;
};

// -----------------------------------------------------------------------------
// Manage an OpenGL context
class RenderContext {
    class Impl;
    Impl * _impl;

public:
    using WindowHandle = intptr_t;

    enum Type {
        STANDALONE,
        SHARED,
    };

    RenderContext(Type type = STANDALONE, WindowHandle externalWindow = 0);
    ~RenderContext();

    LGI_NO_COPY(RenderContext);

    // can move
    RenderContext(RenderContext && that): _impl(that._impl) { that._impl = nullptr; }
    RenderContext & operator=(RenderContext && that);

    void swapBuffers();

    void makeCurrent();

    // unbound render context from current thread.
    static void clearCurrent();
};

// Store and restore OpenGL context
class RenderContextStack {
    class Impl;
    Impl * _impl;

public:
    LGI_NO_COPY(RenderContextStack);
    LGI_NO_MOVE(RenderContextStack);

    RenderContextStack();

    // the destructor will automatically pop out any previously pushed context.
    ~RenderContextStack();

    // push store current context to the top of the stack
    void push();

    // apply previous stored context without pop it out of the stack.
    void apply();

    // pop previously stored context
    void pop();
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace LITESPD_GL_NAMESPACE

#endif // LITESPD_GL_H

#ifdef LITESPD_GL_IMPLEMENTATION
#include "litespd-gl.cpp"
#endif
