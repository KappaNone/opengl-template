#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

char *file_into_malloced_cstr(const char *file_path) {
    FILE *file = NULL;
    char *buffer = NULL;

    file = fopen(file_path, "r"); 
    if (file == NULL) goto fail;
    if (fseek(file, 0, SEEK_END) < 0) goto fail;

    long size = ftell(file);
    if (size < 0) goto fail;

    buffer = malloc(size + 1);
    if (buffer == NULL) goto fail;

    if (fseek(file, 0, SEEK_SET) < 0) goto fail;

    fread(buffer, 1, size, file);
    if (ferror(file)) goto fail;
    
    if (file) {
        fclose(file);
        errno = 0;
    }
    return buffer;
fail:
    if (file) {
        int saved_errno = errno;
        fclose(file);
        errno = saved_errno;
    }
    if (buffer) {
        free(buffer);
    }
    return NULL;
}

const char *shader_type_as_cstr(GLuint shader) {
    switch (shader) {
        case (GL_FRAGMENT_SHADER):
            return "GL_FRAGMENT_SHADER";
        case (GL_VERTEX_SHADER):
            return "GL_VERTEX_SHADER";
        default:
            return "(Undefined)";
    }
}

bool compile_shader_source(const GLchar *source, GLenum shader_type, GLuint *shader) {
    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);

    GLint compiled = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLchar message[1024];
        GLsizei message_size;
        glGetShaderInfoLog(*shader, sizeof(message), &message_size, message);
        fprintf(stderr, "ERROR: could not compile %s\n", shader_type_as_cstr(shader_type));
        fprintf(stderr, "%.*s\n", message_size, message);
        return false;
    }

    return true;
}

bool compile_shader_file(const char *file_path, GLenum shader_type, GLuint *shader) {
    char *source = file_into_malloced_cstr(file_path);
    if (source == NULL) {
        fprintf(stderr, "ERROR: failed to read file `%s` : `%s`\n", file_path, strerror(errno));
        errno = 0;
        return false;
    }

    bool ok = compile_shader_source(source, shader_type, shader);
    if (!ok) {
        fprintf(stderr, "ERROR: failed to compile `%s` shader file\n", file_path);
    }
    free(source);

    return true;
}


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
    GLuint vertex_shader = 0;

    compile_shader_file("shaders/shader.vert", GL_VERTEX_SHADER, &vertex_shader);

    GLuint fragment_shader = 0;

    compile_shader_file("shaders/shader.frag", GL_FRAGMENT_SHADER, &fragment_shader);

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
