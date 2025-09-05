#ifdef WIN32
#include <GL/glew.h>
#include <glfw/glfw3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <glfw/glfw3.h>
#endif
#include <stdio.h>
#include "Application.h"
#include "freeimage.h"

void PrintOpenGLVersion();


int main() {
    FreeImage_Initialise();
    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    const int WindowWidth = 800;
    const int WindowHeight = 600;

    // --- Vollbild: nativer Modus des Hauptmonitors nutzen -------------------- // CHANGED
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(
        mode ? mode->width : WindowWidth,
        mode ? mode->height : WindowHeight,
        "Computergrafik - Hochschule Osnabrück",
        monitor,           //(monitor != nullptr => echtes Fullscreen)
        NULL
    );                                                                                    
    // ------------------------------------------------------------------------ 

    if (!window) {
        fprintf(stderr, "ERROR: can not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

#if WIN32
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    // Maus einsperren, damit Bewegungen nicht am Rand hängen bleiben          // CHANGED
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);               // CHANGED
    // Tipp: Zum Freigeben ggf. spaeter GLFW_CURSOR_NORMAL setzen.             // CHANGED

    PrintOpenGLVersion();

    {
        double lastTime = 0;
        Application App(window);
        App.start();
        while (!glfwWindowShouldClose(window)) {
            double now = glfwGetTime();
            double delta = now - lastTime;
            lastTime = now;
            // once per frame
            glfwPollEvents();
            App.update((float)delta);
            App.draw();
            glfwSwapBuffers(window);
        }
        App.end();
    }

    glfwTerminate();
    return 0;
}


void PrintOpenGLVersion()
{
    // get version info
    const GLubyte* renderer = (const GLubyte*)glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = (const GLubyte*)glGetString(GL_VERSION);  // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);
}
