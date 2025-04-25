
function (add_sample name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} litespd-gl-static)
endfunction()

add_sample(hello-world)
add_sample(simple-triangle)
