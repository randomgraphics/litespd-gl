# setup list of system specific dependencies
if (ANDROID)
    link_libraries(log)
else()
    # build glfw
    function(build_glfw)
        # Enclose build process inside a function. So we can freely change build
        # options w/o affecting the rest of the project
        remove_definitions()
        option(GLFW_BUILD_EXAMPLES "" OFF)
        option(GLFW_BUILD_TESTS "" OFF)
        option(GLFW_BUILD_DOCS "" OFF)
        option(GLFW_INSTALL "" OFF)
        option(GLFW_BUILD_WAYLAND "" OFF)
        add_subdirectory(3rd-party/glfw)
    endfunction()
    build_glfw()
    link_libraries(glfw)
endif()

# build glad
function(build_glad)
    # Enclose build process inside a function. So we can freely change build
    # options w/o affecting the rest of the project
    add_library(glad STATIC 3rd-party/glad/src/glad.c)
    target_include_directories(glad PUBLIC 3rd-party/glad/include)
endfunction()
build_glad()
link_libraries(glad)

# Setup common build flags form samples and tests
if (MSVC)
    add_compile_options(/MP /W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wconversion -Wshadow -Werror -Wfatal-errors)
endif()
include_directories(../inc 3rd-party 3rd-party/glm)
add_compile_definitions($<$<CONFIG:Debug>:LITESPD_GL_ENABLE_DEBUG_BUILD=1>)
set(CMAKE_CXX_STANDARD 17)

# Build litespd-gl into a static library
add_library(litespd-gl-static STATIC lgl.cpp)
link_libraries(litespd-gl-static)

# Build subfolders
add_subdirectory(sample)
add_subdirectory(test)
