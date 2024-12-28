#pragma once
#ifndef GL_SHADER_H
#define GL_SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class GLWindow
{
public:
    GLWindow(int width, int height, const char* title);

    ~GLWindow();

    void MainLoop();

private:
    GLFWwindow* window;
};

#endif // !GL_SHADER_H
