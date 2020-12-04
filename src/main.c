#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>
#include <linmath/linmath.h>

#include "model.h"
#include "opengl.h"
#include "shader.h"
#include "texture.h"

#include "models/sprite.h"
#include "shaders/demo_vert.h"
#include "shaders/demo_frag.h"
#include "sounds/clink.h"
#include "sounds/death.h"
#include "sounds/flap.h"
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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "GLFW3 OpenGL Demo", NULL, NULL);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int prog = shader_compile_and_link(SHADER_DEMO_VERT_SOURCE, SHADER_DEMO_FRAG_SOURCE);
    int u_layer = glGetUniformLocation(prog, "u_layer");
    int u_model = glGetUniformLocation(prog, "u_model");
    int u_projection = glGetUniformLocation(prog, "u_projection");
    // manually set texture uniform location
    glUseProgram(prog);
    glUniform1i(glGetUniformLocation(prog, "u_texture"), 0);
    glUseProgram(0);

    unsigned int vbo = model_buffer_create(MODEL_SPRITE_FORMAT, MODEL_SPRITE_COUNT, MODEL_SPRITE_VERTICES);
    unsigned int vao = model_buffer_config(MODEL_SPRITE_FORMAT, vbo);
    unsigned int tex = texture_create(TEXTURE_BIRD_FORMAT, TEXTURE_BIRD_WIDTH, TEXTURE_BIRD_HEIGHT, TEXTURE_BIRD_PIXELS);

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

        // clear the screen
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind the shader and update uniform value
        glUseProgram(prog);
        glUniform1f(u_layer, 0.0f);

        mat4x4 m = { 0 };
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, angle);
        glUniformMatrix4fv(u_model, 1, GL_FALSE, (const float*)m);

        mat4x4 p = { 0 };
        mat4x4_identity(p);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width/height;
        mat4x4_ortho(p, -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(u_projection, 1, GL_FALSE, (const float*)p);

        // bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        // bind the model and make the draw call
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, MODEL_SPRITE_COUNT);

        // unbind everything
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
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
    glDeleteTextures(1, &tex);

    // Cleanup GLFW3 resources
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
