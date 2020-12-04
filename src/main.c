#include <assert.h>
#include <math.h>
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

// game resources
#include "models/sprite.h"
#include "shaders/sprite_vert.h"
#include "shaders/sprite_frag.h"
#include "sounds/death.h"
#include "sounds/flap.h"
#include "textures/bg.h"
#include "textures/bird.h"
#include "textures/pipe_bot.h"
#include "textures/pipe_top.h"

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

// sprite drawing:
// * has a fixed shader with 2 uniforms: mat4 model, mat4 projection
// * has a fixed unit [-1,1] model with format: T2F_V3F
static void
draw_sprite(int u_model, unsigned texture, float x, float y, float z, float rotate, float scale_x, float scale_y)
{
    // setup model matrix
    mat4x4 m = { 0 };
    mat4x4_translate(m, x, y, z);
    mat4x4_rotate_Z(m, m, rotate*(M_PI/180.0));
    mat4x4_scale_aniso(m, m, scale_x, scale_y, 1.0f);
    glUniformMatrix4fv(u_model, 1, GL_FALSE, (const float*)m);

    // bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // bind the model and make the draw call
    glDrawArrays(GL_TRIANGLES, 0, MODEL_SPRITE_COUNT);  // TODO not pure
}

int
main(int argc, char* argv[])
{
    bool fullscreen = false;
    bool resizable = false;
    bool vsync = false;

    // process CLI args and update corresponding flags
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

    // ask for an OpenGL 3.3 Core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Flappy Bird", NULL, NULL);
    if (window == NULL) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to create GLFW3 window: %s\n", error);

        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(vsync ? 1 : 0);
    opengl_load_functions();

    printf("OpenGL Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL Version:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // enable depth testing (to simulate layers in 2D space)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // create shader for rendering sprites
    unsigned int shader_sprite = shader_compile_and_link(SHADER_SPRITE_VERT_SOURCE, SHADER_SPRITE_FRAG_SOURCE);
    int u_model = glGetUniformLocation(shader_sprite, "u_model");
    int u_projection = glGetUniformLocation(shader_sprite, "u_projection");

    // manually set texture uniform location
    glUseProgram(shader_sprite);
    glUniform1i(glGetUniformLocation(shader_sprite, "u_texture"), 0);
    glUseProgram(0);

    // create model for rendering sprites
    unsigned int buffer_sprite = model_buffer_create(MODEL_SPRITE_FORMAT, MODEL_SPRITE_COUNT, MODEL_SPRITE_VERTICES);
    unsigned int model_sprite = model_buffer_config(MODEL_SPRITE_FORMAT, buffer_sprite);

    // create sprite textures
    unsigned int texture_bg = texture_create(TEXTURE_BG_FORMAT, TEXTURE_BG_WIDTH, TEXTURE_BG_HEIGHT, TEXTURE_BG_PIXELS);
    unsigned int texture_bird = texture_create(TEXTURE_BIRD_FORMAT, TEXTURE_BIRD_WIDTH, TEXTURE_BIRD_HEIGHT, TEXTURE_BIRD_PIXELS);
    unsigned int texture_pipe_bot = texture_create(TEXTURE_PIPE_BOT_FORMAT, TEXTURE_PIPE_BOT_WIDTH, TEXTURE_PIPE_BOT_HEIGHT, TEXTURE_PIPE_BOT_PIXELS);
    unsigned int texture_pipe_top = texture_create(TEXTURE_PIPE_TOP_FORMAT, TEXTURE_PIPE_TOP_WIDTH, TEXTURE_PIPE_TOP_HEIGHT, TEXTURE_PIPE_TOP_PIXELS);

    // bookkeeping vars
    double last_second = glfwGetTime();
    long frame_count = 0;

    // loop til exit or ESCAPE key
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // check window size and set viewport every frame (is this bad?)
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // clear the screen
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind the shader and update uniform value
        glUseProgram(shader_sprite);

        // setup perspective matrix
        mat4x4 p = { 0 };
        mat4x4_identity(p);
        float aspect = (float)width/height;
        mat4x4_ortho(p, -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(u_projection, 1, GL_FALSE, (const float*)p);

        // bind the model and make the draw call
        glBindVertexArray(model_sprite);

        // draw sprites (model, tex, x, y, z, r, sx, xy)
        draw_sprite(u_model, texture_bg, -1.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);
        draw_sprite(u_model, texture_bg, -1.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);
        draw_sprite(u_model, texture_bg, -0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);
        draw_sprite(u_model, texture_bg, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);
        draw_sprite(u_model, texture_bg, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);
        draw_sprite(u_model, texture_bg, 1.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);
        draw_sprite(u_model, texture_bg, 1.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);
        draw_sprite(u_model, texture_pipe_top, 0.0f, 0.7f, 0.1f, 0.0f, 0.1f, 0.5f);
        draw_sprite(u_model, texture_pipe_bot, 0.0f, -0.7f, 0.1f, 0.0f, 0.1f, 0.5f);
        draw_sprite(u_model, texture_bird, sin(glfwGetTime()), cos(glfwGetTime())/2.0, 0.2f, glfwGetTime()*180.0/M_PI, 0.1f, 0.1f);

        // unbind everything
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        // http://www.opengl-tutorial.org/miscellaneous/an-fps-counter/
        double now = glfwGetTime();
        frame_count++;
        if (now - last_second >= 1.0) {
            printf("FPS: %ld  (%lf ms/frame)\n", frame_count, 1000.0/frame_count);
            frame_count = 0;
            last_second += 1.0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup OpenGL resources
    glDeleteVertexArrays(1, &model_sprite);
    glDeleteBuffers(1, &buffer_sprite);
    glDeleteProgram(shader_sprite);
    glDeleteTextures(1, &texture_bg);
    glDeleteTextures(1, &texture_bird);
    glDeleteTextures(1, &texture_pipe_bot);
    glDeleteTextures(1, &texture_pipe_top);

    // Cleanup GLFW3 resources
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
