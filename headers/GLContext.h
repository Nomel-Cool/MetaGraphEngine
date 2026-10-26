#pragma once
#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
class GLContext
{
public:
    GLContext();
    ~GLContext();
    void SetWinSize(int width, int height);
    void SetWinTitle(std::string title);
    bool GenGLWindow();
    void SetFlexibleView();
    void EnableInputControlWindowClosure();
    void SwapBuffers();
    void PollEvents();
    bool ActivateWindow();
    bool DoesWindowAboutToClose();
    const GLFWwindow* GetWinPtr() const;
    void CloseWindow();
private:
    GLFWwindow* p_window;
    int win_width = 640, win_height = 480;
    std::string win_title = "Default Title";
};
#endif // !GL_CONTEXT_H

void framebuffer_size_callback(GLFWwindow* window, int width, int height);