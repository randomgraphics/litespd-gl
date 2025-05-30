#include "lgl.h"
#define LITESPD_GL_IMPLEMENTATION
#include <litespd-gl/litespd-gl.h>

#ifdef __ANDROID__
#include <unwind.h>
#include <cxxabi.h>
#else
#if defined(_MSC_VER)
// disable some backward.hpp warnings reporeted by MSVC
#pragma warning(disable : 4996) // 'strncpy': This function or variable may be unsafe. Consider using strncpy_s instead.
#pragma warning(disable : 4267) // conversion from 'size_t' to 'unsigned int', possible loss of data
#endif
#include "3rd-party/backward-cpp/backward.hpp"
#endif

// ---------------------------------------------------------------------------------------------------------------------
//
std::string backtrace() {
#ifdef __ANDROID__
    struct android_backtrace_state {
        void ** current;
        void ** end;

        static _Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context * context, void * arg) {
            android_backtrace_state * state = (android_backtrace_state *) arg;
            uintptr_t                 pc    = _Unwind_GetIP(context);
            if (pc) {
                if (state->current == state->end)
                    return _URC_END_OF_STACK;
                else
                    *state->current++ = reinterpret_cast<void *>(pc);
            }
            return _URC_NO_REASON;
        }

        static std::string addr2symbol(const void * addr) {
            // convert address to symbol
            Dl_info info;
            if (!dladdr(addr, &info) || !info.dli_sname) return {};
            std::string result = info.dli_sname;

            // demangle c++ syntax
            int    status = 0;
            char * symbol = abi::__cxa_demangle(info.dli_sname, 0, 0, &status);
            if (!symbol) return result;
            if (0 == status) result = symbol;
            free(symbol);

            // done
            return result;
        }
    };

    std::stringstream ss;
    ss << "android stack dump\n";

    const int max = 100;
    void *    buffer[max];

    android_backtrace_state state;
    state.current = buffer;
    state.end     = buffer + max;

    _Unwind_Backtrace(android_backtrace_state::android_unwind_callback, &state);

    int count = (int) (state.current - buffer);

    for (int idx = 0; idx < count; idx++) {
        auto addr   = buffer[idx];
        auto symbol = android_backtrace_state::addr2symbol(addr);
        if (symbol.empty()) symbol = "<no symbol>";
        ss << LITESPD_GL_NAMESPACE::format("%03d: 0x%p %s\n", idx, addr, symbol.c_str());
    }

    ss << "android stack dump done\n";

    return ss.str();
#else
    using namespace backward;
    StackTrace st;
    st.load_here(32);
    std::stringstream ss;
    Printer           p;
    p.snippet = true; // print code snippet in debug build only.
    p.print(st, ss);
    return ss.str();
#endif
}
