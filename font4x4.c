#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

#include "src/opengl.h"

#include "shaders/font_frag.h"
#include "shaders/font_vert.h"

// Building:
// gcc font4x4.c src/opengl.c -std=c99 -DGLFW_INCLUDE_NONE -Ires/ -Isrc/ -Ivendor/include -lGL -lglfw

/*
0000 0
0001 1
0010 2
0011 3 
0100 4
0101 5
0110 6
0111 7
1000 8
1001 9
1010 a
1011 b
1100 c
1101 d
1110 e
1111 f

0 -> 0xeae0:

000.
0.0.
000.
....

*/

// each nibble is a line from top to bottom
// each character has a width and height of 1 unit
// a chararacter's x and y are in the center
// kerning is built into the font
// each character has 2 triangles per lit pixel
// each character has 6 vertices per lit pixel
// each vertex has 3 floats (x, y, z)
// each lit pixel occupies 18 floats (72 bytes)
static const unsigned short font[] = {
    0xeae0,  // 0
    0x4430,  // 1
    0x2ce0,  // 2
    0xe6e0,  // 3
    0xae20,  // 4
    0xec20,  // 5
    0x8ae0,  // 6
    0xe220,  // 7
    0xeee0,  // 8
    0xea20,  // 9
};

enum {
    FLOATS_PER_QUAD = 12,
};

static const float quads[][FLOATS_PER_QUAD] = {
    //      top right       top left        bottom left     top right       bottom left     bottom right
    //      x       y       x       y       x       y       x       y       x       y       x       y
    [0] = {  0.50f, -0.25f,  0.25f, -0.25f,  0.25f, -0.50f,  0.50f, -0.25f,  0.25f, -0.50f,  0.50f, -0.50f },
    [1] = { -0.00f, -0.00f, -0.00f, -0.00f, -0.00f, -0.00f, -0.00f, -0.00f, -0.00f, -0.00f, -0.00f, -0.00f },
};

long
font_size(const char* s)
{
    char c;
    while ((c = *s++) != '\0') {
        c = c - '0';
        assert(c >= 0 && c <= 9);

        unsigned short glyph = font[c];
        printf("0x%04x\n", glyph);
        for (long i = 0; i < 4*4; i++) {
            char pixel = (glyph >> i) & 1;
            printf("pixel: %d\n", pixel);
        }
    }
}


int
main(int argc, char* argv[])
{
    if (!glfwInit()) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to init GLFW3: %s\n", error);
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "4x4 Font Example", NULL, NULL);
    if (window == NULL) {
        const char* error = NULL;
        glfwGetError(&error);
        fprintf(stderr, "failed to create GLFW3 window: %s\n", error);

        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    opengl_load_functions();

    font_size("042");

    printf("OpenGL Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL Version:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glViewport(0.0f, 0.0f, 1280.0f, 720.0f);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } 

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
