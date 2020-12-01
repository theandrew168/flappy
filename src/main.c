#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "opengl.h"

#include "models/square.h"
#include "shaders/bg_vert.h"
#include "shaders/bg_frag.h"
#include "shaders/demo_vert.h"
#include "shaders/demo_frag.h"
#include "textures/bg.h"
#include "textures/bird.h"
#include "textures/pipe.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif


static void
print_usage(const char* arg0)
{
    printf("usage: %s [options]\n", arg0);
    printf("\n");
    printf("Options:\n");
    printf("  -h --help        print this help\n");
    printf("  -f --fullscreen  fullscreen window\n");
    printf("  -r --resizable   resizable window\n");
    printf("  -v --vsync       enable vsync\n");
}

static void
framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int
main(int argc, char* argv[])
{
    bool fullscreen = false;
    bool resizable = false;
    bool vsync = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fullscreen") == 0) {
            fullscreen = true;
        }
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--resizable") == 0) {
            resizable = true;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vsync") == 0) {
            vsync = true;
        }
    }

    if (!glfwInit()) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to init GLFW3: %s\n", error);
        return EXIT_FAILURE;
    }

    // use "fake" fullscreen
    if (fullscreen) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }

    glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(640, 640, "GLFW3 OpenGL Demo", NULL, NULL);
    if (window == NULL) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to create GLFW3 window: %s\n", error);

        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Enable sticky keys
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Enable v-sync (set 1 to enable, 0 to disable)
    glfwSwapInterval(vsync ? 1 : 0);

    // Load the modern OpenGL funcs
    opengl_load_functions();

    // Print some debug OpenGL info
    printf("OpenGL Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL Version:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Call glViewport when window gets resized
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Do modern OpenGL stuff
    int vs = shader_compile_source(SHADER_DEMO_VERT_TYPE, SHADER_DEMO_VERT_SOURCE, SHADER_DEMO_VERT_LENGTH);
    int fs = shader_compile_source(SHADER_DEMO_FRAG_TYPE, SHADER_DEMO_FRAG_SOURCE, SHADER_DEMO_FRAG_LENGTH);
    int prog = shader_link_program(vs, fs);

    GLint uniform_angle = glGetUniformLocation(prog, "angle");

    glDeleteShader(fs);
    glDeleteShader(vs);


    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MODEL_SQUARE_VERTICES), MODEL_SQUARE_VERTICES, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    long frame_count = 0;
    unsigned long last_frame = 0;
    unsigned long last_second = 0;
    double angle = 0.0;

    // Set the OpenGL viewport size
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Loop til exit or ESCAPE key
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glUniform1f(uniform_angle, angle);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, MODEL_SQUARE_COUNT);
        glBindVertexArray(0);
        glUseProgram(0);

        // glfwGetTime returns seconds, convert to milliseconds
        unsigned long now = (unsigned long)(glfwGetTime() * 1000.0);
        unsigned long diff = now - last_frame;
        last_frame = now;

        angle += diff / 1000.0;
        if (angle > 2 * M_PI) angle -= 2 * M_PI;

        if (now - last_second >= 1000) {
            printf("FPS: %ld\n", frame_count);
            frame_count = 0;
            last_second = now;
        }

        frame_count++;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup OpenGL resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);

    // Cleanup GLFW3 resources
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
