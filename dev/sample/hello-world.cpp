#include "../lgl.h"

int main() {
    // Create an OpenGL context with all default options.
    litespd::gl::RenderContext rc;

    // Main loop
    while(rc.beginFrame()) {
        // Clear the screen
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Swap the buffers
        rc.endFrame();
    }
}
