#include "../lgl.h"

struct Scene {
    litespd::gl::SimpleMesh        tri;
    litespd::gl::SimpleGlslProgram program;

    Scene() = default;

    ~Scene() { quit(); }

    bool init() {
        // should have no GL errors.
        LGI_CHK(;);

        // create a triangle mesh
        const litespd::gl::SimpleMesh::Vertex vertices[] = {
            litespd::gl::SimpleMesh::Vertex::create().setPosition({+0.f, +.5f, .0f}).setColor({1.0f, 0.0f, 0.0f, 1.0f}),
            litespd::gl::SimpleMesh::Vertex::create().setPosition({-.5f, -.5f, .0f}).setColor({0.0f, 1.0f, 0.0f, 1.0f}),
            litespd::gl::SimpleMesh::Vertex::create().setPosition({+.5f, -.5f, .0f}).setColor({0.0f, 0.0f, 1.0f, 1.0f}),
        };
        tri.allocate(litespd::gl::SimpleMesh::AllocateParameters().setVertices(std::size(vertices), vertices));

        // create a GPU program
        const char * vs = R"(
            #version 150
            #extension GL_ARB_explicit_attrib_location : enable
            layout(location = 0) in vec4 a_position; // position is at location 0 in SimpleMesh::Vertex
            layout(location = 3) in vec4 a_color; // color is at location 3 in SimpleMesh::Vertex
            out vec4 v_color;
            void main() {
                gl_Position = a_position;
                v_color = a_color;
            }
        )";
        const char * ps = R"(
            #version 150
            precision mediump float;
            in vec4 v_color;
            out vec4 o_color;
            void main() {
                o_color = v_color;
            }
        )";
        program.loadVsPs(vs, ps);
        return true;
    }

    void quit() {
        tri.cleanup();
        program.cleanup();
    }

    void render() {
        program.use();
        tri.draw();
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
