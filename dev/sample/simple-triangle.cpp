#include "../lgl.h"

struct Scene {

    Scene() = default;

    ~Scene() { quit(); }

    bool init() {
        // create a triangle mesh
        return true;
    }

    void quit() {
        //
    }

    void render() {
        //
    }
};

int main() {
    // Create an OpenGL context with all default options.
    litespd::gl::RenderContext rc({});

    Scene s;

    if (!s.init()) return -1;

    // Main loop
    while (rc.beginFrame()) {
        // Clear the screen
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // render the frame
        s.render();

        // Swap the buffers
        rc.endFrame();
    }
}
