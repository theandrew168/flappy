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

struct point {
    float x;
    float y;
};

struct quad {
    struct point p;
    float w;
    float h;
};

struct circle {
    struct point p;
    float r;
};

bool
intersect_point_quad(struct point p, struct quad q)
{
    // calculate edges of the quad
    float left = q.p.x - (q.w / 2.0f);
    float right = q.p.x + (q.w / 2.0f);
    float bottom = q.p.y - (q.h / 2.0f);
    float top = q.p.y + (q.h / 2.0f);

    // intersection if p lies within the quad
    return p.x >= left && p.x <= right && p.y >= bottom && p.y <= top;
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
    mat4x4 m = { 0 };
    mat4x4_translate(m, x, y, z);
    mat4x4_rotate_Z(m, m, r * (M_PI / 180.0));
    mat4x4_scale_aniso(m, m, sx, sy, 1.0f);
    glUniformMatrix4fv(spriter->uniform_model, 1, GL_FALSE, (const float*)m);

    // setup projection matrix
    mat4x4 p = { 0 };
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

struct bird {
    struct point pos;
    struct point vel;
    unsigned int texture;
    struct point scale;
    float layer;
};


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

    srand(time(NULL));

    if (!glfwInit()) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to init GLFW3: %s\n", error);
        return EXIT_FAILURE;
    }

    // TODO: how to fullscreen?
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
    struct bird bird = {
        .pos = { 0.0f, 0.0f },
        .texture = texture_bird,
        .scale = { 0.5f, 0.5f },
        .layer = 0.2f,
    };

    // bookkeeping vars
    double last_second = glfwGetTime();
    long frame_count = 0;

    // loop til exit or ESCAPE key
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

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

        // draw background
        for (float x = -9.0f; x <= 9.0f; x += 4.5f) {
            spriter_draw(&spriter, texture_bg, x, 0.0f, 0.0f, 0.0f, 2.25f, 4.5f);
        }

        // draw bird
        spriter_draw(&spriter, texture_bird, bird.pos.x, bird.pos.y, bird.layer, 0.0f, bird.scale.x, bird.scale.y);

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
