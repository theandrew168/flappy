#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GLFW/glfw3.h>
#include <linmath/linmath.h>

#include "config.h"
#include "model.h"
#include "opengl.h"
#include "physics.h"
#include "shader.h"
#include "texture.h"

// game resources
#include "models/sprite.h"
#include "shaders/sprite_vert.h"
#include "shaders/sprite_frag.h"
#include "textures/bg.h"
#include "textures/bird.h"
#include "textures/pipe_bot.h"
#include "textures/pipe_top.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif


enum {
    PIPE_COUNT = 512,
};

struct game {
    // OpenGL handles for sprite rendering
    unsigned int sprite_shader;
    int sprite_shader_uniform_model;
    int sprite_shader_uniform_projection;
    unsigned int sprite_buffer;
    unsigned int sprite_model;
    unsigned int sprite_model_vertex_count;

    // OpenGL texture handles
    unsigned int texture_bg;
    unsigned int texture_bird;
    unsigned int texture_pipe_bot;
    unsigned int texture_pipe_top;

    // timing vars
    double last_second;
    double last_frame;
    long frame_count;

    // game state
    bool running;
    bool dead;
    bool space;

    // game objects
    float camera;
    float bird_pos_x;
    float bird_pos_y;
    float bird_vel_x;
    float bird_vel_y;
    float pipes[PIPE_COUNT];
};

bool game_init(struct game* game);
void game_free(struct game* game);
void game_reset(struct game* game);
void game_update(struct game* game, GLFWwindow* window, double delta);
void game_render(struct game* game, long width, long height);

static void
draw_sprite(struct game* game, unsigned t, float x, float y, float z, float r, float sx, float sy)
{
    // bind the shader
    glUseProgram(game->sprite_shader);

    // setup model matrix
    mat4x4 m = {{ 0 }};
    mat4x4_translate(m, x, y, z);
    mat4x4_rotate_Z(m, m, r * (M_PI / 180.0));  // convert deg to rad
    mat4x4_scale_aniso(m, m, sx, sy, 1.0f);
    glUniformMatrix4fv(game->sprite_shader_uniform_model, 1, GL_FALSE, (const float*)m);

    // setup projection matrix
    mat4x4 p = {{ 0 }};
    mat4x4_identity(p);
    mat4x4_ortho(p, -(WIDTH / 2.0f), (WIDTH / 2.0f), -(HEIGHT / 2.0f), (HEIGHT / 2.0f), -1.0f, 1.0f);
    glUniformMatrix4fv(game->sprite_shader_uniform_projection, 1, GL_FALSE, (const float*)p);

    // bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t);

    // bind the model
    glBindVertexArray(game->sprite_model);

    // draw the sprite!
    glDrawArrays(GL_TRIANGLES, 0, game->sprite_model_vertex_count);

    // unbind everything
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

bool
game_init(struct game* game)
{
    assert(game != NULL);

    // create shader for rendering sprites
    game->sprite_shader = shader_compile_and_link(SHADER_SPRITE_VERT_SOURCE, SHADER_SPRITE_FRAG_SOURCE);
    game->sprite_shader_uniform_model = glGetUniformLocation(game->sprite_shader, "u_model");
    game->sprite_shader_uniform_projection = glGetUniformLocation(game->sprite_shader, "u_projection");

    // set texture uniform location
    glUseProgram(game->sprite_shader);
    glUniform1i(glGetUniformLocation(game->sprite_shader, "u_texture"), 0);
    glUseProgram(0);

    // create model for rendering sprites
    game->sprite_buffer = model_buffer_create(MODEL_SPRITE_FORMAT, MODEL_SPRITE_VERTEX_COUNT, MODEL_SPRITE_VERTICES);
    game->sprite_model = model_buffer_config(MODEL_SPRITE_FORMAT, game->sprite_buffer);
    game->sprite_model_vertex_count = MODEL_SPRITE_VERTEX_COUNT;

    // create textures
    game->texture_bg = texture_create(TEXTURE_BG_FORMAT, TEXTURE_BG_WIDTH, TEXTURE_BG_HEIGHT, TEXTURE_BG_PIXELS);
    game->texture_bird = texture_create(TEXTURE_BIRD_FORMAT, TEXTURE_BIRD_WIDTH, TEXTURE_BIRD_HEIGHT, TEXTURE_BIRD_PIXELS);
    game->texture_pipe_bot = texture_create(TEXTURE_PIPE_BOT_FORMAT, TEXTURE_PIPE_BOT_WIDTH, TEXTURE_PIPE_BOT_HEIGHT, TEXTURE_PIPE_BOT_PIXELS);
    game->texture_pipe_top = texture_create(TEXTURE_PIPE_TOP_FORMAT, TEXTURE_PIPE_TOP_WIDTH, TEXTURE_PIPE_TOP_HEIGHT, TEXTURE_PIPE_TOP_PIXELS);

    // reset
    game_reset(game);
    return true;
}

void
game_free(struct game* game)
{
    assert(game != NULL);

    glDeleteProgram(game->sprite_shader);
    glDeleteBuffers(1, &game->sprite_buffer);
    glDeleteVertexArrays(1, &game->sprite_model);
    glDeleteTextures(1, &game->texture_bg);
    glDeleteTextures(1, &game->texture_bird);
    glDeleteTextures(1, &game->texture_pipe_bot);
    glDeleteTextures(1, &game->texture_pipe_top);
}

void
game_reset(struct game* game)
{
    assert(game != NULL);

    // game state
    game->running = false;
    game->dead = false;
    game->space = false;

    // game objects
    game->camera = -3.0f;
    game->bird_pos_x = -6.0f;
    game->bird_pos_y = 0.0f;
    game->bird_vel_x = SPEED;
    game->bird_vel_y = 0.0f;
    for (long i = 0; i < PIPE_COUNT; i++) {
        float gap = (float)rand() / (float)RAND_MAX;  // [0.0, 1.0]
        gap -= 0.5f;  // [-0.5, 0.5]
        game->pipes[i] = gap * 4.0f;  // [-2.0, 2.0]
    }
}

void
game_update(struct game* game, GLFWwindow* window, double delta)
{
    // can game_update be detached from the GLFWwindow* ?
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // only allow single flaps (not continuous)
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (game->dead) game_reset(game);

        game->running = true;
        if (!game->space) {
            game->bird_vel_y = FLAP;
            game->space = true;
        } else {
            game->bird_vel_y -= delta * GRAVITY;
        }
    } else {
        if (game->running) {
            game->bird_vel_y -= delta * GRAVITY;
        }
        game->space = false;
    }

    // update bird and camera positions
    if (game->running) {
        game->bird_pos_x += (game->bird_vel_x * delta);
        game->bird_pos_y += (game->bird_vel_y * delta);
        game->camera += (game->bird_vel_x * delta);
    }

    // check collision
    if (game->bird_pos_x >= -4.0f) {
        // determine index of the next approaching pipe
        long pipe_index = (game->bird_pos_x + 2.0f) / 4.0f;
        float gap = game->pipes[pipe_index % PIPE_COUNT];
        float top = gap + GAP;
        float bot = gap - GAP;

        bool collision = false;
        if (physics_intersect_circle_rect(game->bird_pos_x, game->bird_pos_y, 0.3f,
                                          pipe_index * 4.0f, top, PIPE_WIDTH, PIPE_HEIGHT)) {
            collision = true;
        }
        if (physics_intersect_circle_rect(game->bird_pos_x, game->bird_pos_y, 0.3f,
                                          pipe_index * 4.0f, bot, PIPE_WIDTH, PIPE_HEIGHT)) {
            collision = true;
        }
        if (game->bird_pos_y > 4.5f || game->bird_pos_y < -4.5f) {
            collision = true;
        }

        if (collision && !game->dead) {
            game->dead = true;
            game->bird_vel_x = 0.0f;
            game->bird_vel_y = 8.0f;

            // determine score based on bird's position
            long score = (game->bird_pos_x + 3.0f) / 4.0f;
            printf("%s\n", "YOU DIED!");
            printf("Score: %ld\n", score);
        }
    }
}

void
game_render(struct game* game, long width, long height)
{
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
    double bg_scroll = glfwGetTime() * SCROLL;
    double bg_offset = fmod(bg_scroll, 4.5);
    for (float x = -9.0f; x <= 13.5f; x += 4.5f) {
        draw_sprite(game, game->texture_bg,
            x - bg_offset, 0.0f, BG_LAYER,
            0.0f, BG_WIDTH, BG_HEIGHT);
    }

    // draw pipes (every 4.0f units starting at 0.0f)
    for (float x = game->camera - 8.0f; x <= game->camera + 12.0f; x += 4.0f) {
        if (x < 0.0f) continue;
        long pipe_index = x / 4.0f;
        float gap = game->pipes[pipe_index % PIPE_COUNT];
        float top = gap + GAP;
        float bot = gap - GAP;
        float pipe_x = pipe_index * 4.0f;
        draw_sprite(game, game->texture_pipe_top,
            pipe_x - game->camera, top, PIPE_LAYER,
            0.0f, PIPE_WIDTH, PIPE_HEIGHT);
        draw_sprite(game, game->texture_pipe_bot,
            pipe_x - game->camera, bot, PIPE_LAYER,
            0.0f, PIPE_WIDTH, PIPE_HEIGHT);
    }

    // draw bird
    draw_sprite(game, game->texture_bird,
        game->bird_pos_x - game->camera, game->bird_pos_y, BIRD_LAYER,
        game->bird_vel_y * 5.0f, BIRD_WIDTH, BIRD_HEIGHT);
}




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

    struct game game = { 0 };
    game_init(&game);

    // timing vars
    double last_second = glfwGetTime();
    double last_frame = last_second;
    long frame_count = 0;

    // loop til exit or ESCAPE key
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double delta = now - last_frame;
        last_frame = now;

        game_update(&game, window, delta);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        game_render(&game, width, height);

        frame_count++;
        if (glfwGetTime() - last_second >= 1.0) {
            printf("FPS: %ld  (%lf ms/frame)\n", frame_count, 1000.0/frame_count);
            frame_count = 0;
            last_second += 1.0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    game_free(&game);

    // Cleanup GLFW3 resources
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
