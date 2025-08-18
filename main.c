#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

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

    buffer[size] = '\0';
    
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

bool compile_shader_source(const GLchar *source, GLuint *shader, GLenum shader_type) {
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

bool compile_shader_file(const char *file_path, GLuint *shader, GLenum shader_type) {
    char *source = file_into_malloced_cstr(file_path);
    if (source == NULL) {
        fprintf(stderr, "ERROR: failed to read file `%s` : `%s`\n", file_path, strerror(errno));
        errno = 0;
        return false;
    }

    bool ok = compile_shader_source(source, shader, shader_type);
    if (!ok) {
        fprintf(stderr, "ERROR: failed to compile `%s` shader file\n", file_path);
    }

    free(source);
    return true;
}

bool link_program(GLuint vert_shader, GLuint frag_shader, GLuint *program) {
    *program = glCreateProgram(); 

    glAttachShader(*program, vert_shader);
    glAttachShader(*program, frag_shader);
    glLinkProgram(*program);
    
    GLint linked = 0;
    glGetProgramiv(*program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLsizei message_size = 0;
        GLchar message[1024];

        glGetProgramInfoLog(*program, sizeof(message), &message_size, message);
        fprintf(stderr, "Program linking: %.*s\n", message_size, message);
    }

    glDeleteShader(vert_shader); 
    glDeleteShader(frag_shader); 

    return program;
};

double time = 0;

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello from OpenGL!", 0, 0);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    float vertices[] = {
        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // top-left
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, // top_right
        1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // bottom-right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-left
    };

    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0,
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

    // Creating ebo (elements buffer object) to reuse vertices using elements[]
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Compiling shaders
    GLuint vert_shader = 0;

    compile_shader_file("shaders/shader.vert", &vert_shader, GL_VERTEX_SHADER);

    GLuint frag_shader = 0;

    compile_shader_file("shaders/blink.frag", &frag_shader, GL_FRAGMENT_SHADER);

    // Linking shaders into a program 
    GLuint program;

    link_program(vert_shader, frag_shader, &program);

    glUseProgram(program);

    // Making the link between vertex data and attributes
    GLint pos_attrib = glGetAttribLocation(program, "pos");
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(pos_attrib);

    GLint color_attrib = glGetAttribLocation(program, "color");
    glVertexAttribPointer(color_attrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(color_attrib);
    
    // Uniforms
    GLint uni_time = glGetUniformLocation(program, "time");

    // Main loop
    time = glfwGetTime();
    double prev_time = 0.0f;
    double delta_time = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glUniform1f(uni_time, time);

        double cur_time = glfwGetTime();
        delta_time = cur_time - prev_time;
        time += delta_time;
        prev_time = cur_time;
    }

    glfwTerminate();
    return 1;
}
