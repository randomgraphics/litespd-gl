#include "../lgl.h"

int main() {
    // Create an standard OpenGL context
    litespd::gl::RenderContext rc;

    rc.makeCurrent();

    while(true) {
        // Clear the screen
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap the buffers
        rc.swapBuffers();
    }
}
