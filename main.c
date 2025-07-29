#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

const char *vertex_shader_src = 
"#version 150 core\n"
"in vec2 pos;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(pos, 0.0, 1.0);\n"
"};\n";

const char *fragment_shader_src = 
"#version 150 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Hello from OpenGL!", 0, 0);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    float vertices[] = {
        0.0f, 0.5f,
        0.5f, -0.5f,
        -0.5f, -0.5f,
    };

    // Creating vbo (vertex buffer object) for storing vertex data
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Creating vao (vertex array object) 
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Compiling shaders
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, 0); 
    glCompileShader(vertex_shader);

    GLint shader_compile_status;
    char error_buf[512];

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_compile_status);
    if (shader_compile_status != GL_TRUE) {
        glGetShaderInfoLog(vertex_shader, 512, 0, error_buf);
        printf("VERTEX SHADER COMPILATION ERROR: \n");
        printf("%s\n", error_buf);
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, 0); 
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_compile_status);
    if (shader_compile_status != GL_TRUE) {
        glGetShaderInfoLog(fragment_shader, 512, 0, error_buf);
        printf("FRAGMENT SHADER COMPILATION ERROR: \n");
        printf("%s\n", error_buf);
    }

    // Combining shaders into a program 
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    // Making the link between vertex data and attributes
    GLint pos_attrib = glGetAttribLocation(shader_program, "pos");
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(pos_attrib);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glfwTerminate();
    return 1;
}
