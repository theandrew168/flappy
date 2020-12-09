#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

// game is designed for a 16:9 aspect ratio
static const float WIDTH = 16.0f;
static const float HEIGHT = 9.0f;
static const float ASPECT = WIDTH / HEIGHT;

// Based on:
// http://www.jeffreythompson.org/collision-detection/circle-rect.php
//  modified for rx and ry being in the center of the rect
static bool
intersect_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh)
{
    float test_x = cx;
    float test_y = cy;
    float half_rw = rw / 2.0f;
    float half_rh = rh / 2.0f;

    if (cx < rx - half_rw) test_x = rx - half_rw;  // left edge
    else if (cx > rx + half_rw) test_x = rx + half_rw;  // right edge
    if (cy < ry - half_rh) test_y = ry - half_rh;  // bottom edge
    else if (cy > ry + half_rh) test_y = ry + half_rh;  // top edge

    // check distance from closest edges
    float dist_x = cx - test_x;
    float dist_y = cy - test_y;
    float distance = sqrtf((dist_x * dist_x) + (dist_y * dist_y));

    return distance <= cr;
}

struct spriter {
    unsigned int shader;
    int uniform_model;
    int uniform_projection;
    unsigned int model;
    long vertex_count;
};

static void
spriter_draw(const struct spriter* spriter, unsigned t, float x, float y, float z, float r, float sx, float sy)
{
    // bind the shader
    glUseProgram(spriter->shader);

    // setup model matrix
    mat4x4 m = {{ 0 }};
    mat4x4_translate(m, x, y, z);
    mat4x4_rotate_Z(m, m, r * (M_PI / 180.0));
    mat4x4_scale_aniso(m, m, sx, sy, 1.0f);
    glUniformMatrix4fv(spriter->uniform_model, 1, GL_FALSE, (const float*)m);

    // setup projection matrix
    mat4x4 p = {{ 0 }};
    mat4x4_identity(p);
    mat4x4_ortho(p, -(WIDTH / 2.0f), (WIDTH / 2.0f), -(HEIGHT / 2.0f), (HEIGHT / 2.0f), -1.0f, 1.0f);
    glUniformMatrix4fv(spriter->uniform_projection, 1, GL_FALSE, (const float*)p);

    // bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t);

    // bind the model
    glBindVertexArray(spriter->model);

    // draw the sprite!
    glDrawArrays(GL_TRIANGLES, 0, spriter->vertex_count);

    // unbind everything
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

enum {
    PIPE_COUNT = 512,
};


static void
print_usage(const char* arg0)
{
    printf("usage: %s [options]\n", arg0);
    printf("\n");
    printf("Options:\n");
    printf("  -h --help        print this help\n");
    printf("  -f --fullscreen  fullscreen window\n");
    printf("  -v --vsync       enable vsync\n");
}

int
main(int argc, char* argv[])
{
    bool fullscreen = false;
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
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vsync") == 0) {
            vsync = true;
        }
    }

    srand(time(NULL));

    if (!glfwInit()) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to init GLFW3: %s\n", error);
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // ask for an OpenGL 3.3 Core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    // set flags for monitor details
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    GLFWwindow* window = NULL;
    if (fullscreen) {
        window = glfwCreateWindow(mode->width, mode->height, "Flappy Bird", monitor, NULL);
    } else {
        window = glfwCreateWindow(1280, 720, "Flappy Bird", NULL, NULL);
    }

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // create shader for rendering sprites
    unsigned int shader_sprite = shader_compile_and_link(SHADER_SPRITE_VERT_SOURCE, SHADER_SPRITE_FRAG_SOURCE);
    int u_model = glGetUniformLocation(shader_sprite, "u_model");
    int u_projection = glGetUniformLocation(shader_sprite, "u_projection");

    // set texture uniform location
    glUseProgram(shader_sprite);
    glUniform1i(glGetUniformLocation(shader_sprite, "u_texture"), 0);
    glUseProgram(0);

    // create model for rendering sprites
    unsigned int buffer_sprite = model_buffer_create(MODEL_SPRITE_FORMAT, MODEL_SPRITE_COUNT, MODEL_SPRITE_VERTICES);
    unsigned int model_sprite = model_buffer_config(MODEL_SPRITE_FORMAT, buffer_sprite);

    // collect spriter info
    struct spriter spriter = {
        .shader = shader_sprite,
        .uniform_model = u_model,
        .uniform_projection = u_projection,
        .model = model_sprite,
        .vertex_count = MODEL_SPRITE_COUNT,
    };

    // create sprite textures
    unsigned int texture_bg = texture_create(TEXTURE_BG_FORMAT, TEXTURE_BG_WIDTH, TEXTURE_BG_HEIGHT, TEXTURE_BG_PIXELS);
    unsigned int texture_bird = texture_create(TEXTURE_BIRD_FORMAT, TEXTURE_BIRD_WIDTH, TEXTURE_BIRD_HEIGHT, TEXTURE_BIRD_PIXELS);
    unsigned int texture_pipe_bot = texture_create(TEXTURE_PIPE_BOT_FORMAT, TEXTURE_PIPE_BOT_WIDTH, TEXTURE_PIPE_BOT_HEIGHT, TEXTURE_PIPE_BOT_PIXELS);
    unsigned int texture_pipe_top = texture_create(TEXTURE_PIPE_TOP_FORMAT, TEXTURE_PIPE_TOP_WIDTH, TEXTURE_PIPE_TOP_HEIGHT, TEXTURE_PIPE_TOP_PIXELS);

    // game objects
    float camera = -3.0f;

    float pipes[PIPE_COUNT] = { 0.0f };
    for (long i = 0; i < PIPE_COUNT; i++) {
        float gap = (float)rand() / (float)RAND_MAX;
        gap -= 0.5f;
        pipes[i] = gap * 2.0f;
    }

    float bird_pos_x = -6.0f;
    float bird_pos_y = 0.0f;
    float bird_vel_x = 1.5f;
    float bird_vel_y = 0.0f;

    // bookkeeping vars
    bool dead = false;
    bool space = false;
    double bg_scroll = 0.0f;
    double last_second = glfwGetTime();
    double last_frame = last_second;
    long frame_count = 0;

    // loop til exit or ESCAPE key
    while (!glfwWindowShouldClose(window)) {
        //
        // UPDATE
        //

        double now = glfwGetTime();
        double delta = now - last_frame;
        last_frame = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // only allow single flaps (not continuous)
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !dead) {
            if (!space) {
                bird_vel_y = 8.0f;
                space = true;
            } else {
                bird_vel_y -= delta * 18.0f;
            }
        } else {
            bird_vel_y -= delta * 18.0f;
            space = false;
        }

        // update bird and camera positions
        bird_pos_x += (bird_vel_x * delta);
        bird_pos_y += (bird_vel_y * delta);
        camera += (bird_vel_x * delta);

        // check collision
        if (bird_pos_x >= -4.0f) {
            long pipe_index = (bird_pos_x + 2.0f) / 4.0f;
            float gap = pipes[pipe_index % PIPE_COUNT];
            float top = gap + 6.0f;
            float bot = gap - 6.0f;

            bool collision = false;
            if (intersect_circle_rect(bird_pos_x, bird_pos_y, 0.3f, pipe_index * 4.0f, top, 1.0f, 8.0f)) {
                collision = true;
            }
            if (intersect_circle_rect(bird_pos_x, bird_pos_y, 0.3f, pipe_index * 4.0f, bot, 1.0f, 8.0f)) {
                collision = true;
            }

            if (collision && !dead) {
                dead = true;
                bird_vel_x = 0.0f;
                bird_vel_y = 8.0f;
            }
        }

        //
        // RENDER
        //

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // determine boxing and calculate centering offsets
        long x_offset = 0;
        long y_offset = 0;
        float aspect = (float)width / height;
        if (aspect <= ASPECT) {
            // letterbox
            y_offset = (height - (width / ASPECT)) / 2;
            height = width / ASPECT;
        } else {
            // pillarbox
            x_offset = (width - (height * ASPECT)) / 2;
            width = height * ASPECT;
        }

        // set viewport every frame (is this bad?)
        glViewport(x_offset, y_offset, width, height);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw background (scrolls independently of game objects)
        if (!dead) {
            bg_scroll = glfwGetTime() * 0.5f;
        }
        double bg_offset = fmod(bg_scroll, 4.5);
        for (float x = -9.0f; x <= 13.5f; x += 4.5f) {
            spriter_draw(&spriter, texture_bg, x - bg_offset, 0.0f, 0.0f, 0.0f, 2.25f, 4.5f);
        }

        // draw pipes (every 4.0f units starting at 0.0f)
        for (float x = camera - 8.0f; x <= camera + 12.0f; x += 4.0f) {
            if (x < 0.0f) continue;
            long pipe_index = x / 4.0f;
            float gap = pipes[pipe_index % PIPE_COUNT];
            float top = gap + 6.0f;
            float bot = gap - 6.0f;
            float pipe_x = pipe_index * 4.0f;
            spriter_draw(&spriter, texture_pipe_top, pipe_x - camera, top, 0.1f, 0.0f, 0.5f, 4.0f);
            spriter_draw(&spriter, texture_pipe_bot, pipe_x - camera, bot, 0.1f, 0.0f, 0.5f, 4.0f);
        }

        // draw bird
        spriter_draw(&spriter, texture_bird, bird_pos_x - camera, bird_pos_y, 0.2f, bird_vel_y * 5.0f, 0.5f, 0.5f);

        // http://www.opengl-tutorial.org/miscellaneous/an-fps-counter/
        frame_count++;
        if (glfwGetTime() - last_second >= 1.0) {
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
