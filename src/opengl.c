#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glcorearb.h>
#include <GLFW/glfw3.h>

#include "opengl.h"

// Define an OpenGL function. Until dynamically loaded, it will
// be set to NULL and should NOT be called. Doing so will cause
// a segfault.
//
// OPENGL_DEFINE(glCreateShader, PFNGLCREATESHADERPROC)
//
//   becomes
//
// PFNGLCREATESHADERPROC glCreateShader = NULL;
#define OPENGL_DEFINE(func_name, func_type)  \
    func_type func_name = NULL;

// Define all of the initally-NULL OpenGL functions.
#define OPENGL_FUNCTION OPENGL_DEFINE
OPENGL_FUNCTIONS
#undef OPENGL_FUNCTION

// Load an OpenGL function via glfwGetProcAddress. Check for errors
// and return from the load if something goes wrong. The OpenGL function
// pointer is assigned to the the definition that was initially NULL.
//
// OPENGL_LOAD(glCreateShader, PFNGLCREATESHADERPROC)
//
//   becomes
//
// glCreateShader = (PFNGLCREATESHADERPROC)glfwGetProcAddress("glCreateShader");
#define OPENGL_LOAD(func_name, func_type)  \
    func_name = (func_type)glfwGetProcAddress(#func_name);

// Extra safety step to ensure that all the OpenGL functions were successfully
// dynamically loaded. If a function failed to load, print and error and
// return false back to the caller.
//
// OPENGL_VALIDATE(glCreateShader, PFNGLCREATESHADERPROC)
//
//   becomes
//
// if (glCreateShader == NULL) {
//     fprintf(stderr, "failed to load func: %s\n", "glCreateShader);
//     return false;
// }
#define OPENGL_VALIDATE(func_name, func_type)                      \
    if (func_name == NULL) {                                       \
        fprintf(stderr, "failed to load func: %s\n", #func_name);  \
        return false;                                              \
    }

bool
opengl_load_functions(void)
{
    // use GLFW3's platform-agnostic loader to pull the "real" addresses
    //  out by name and assign to the definitions above

    #define OPENGL_FUNCTION OPENGL_LOAD
    OPENGL_FUNCTIONS
    #undef OPENGL_FUNCTION

    #define OPENGL_FUNCTION OPENGL_VALIDATE
    OPENGL_FUNCTIONS
    #undef OPENGL_FUNCTION

    return true;
}
