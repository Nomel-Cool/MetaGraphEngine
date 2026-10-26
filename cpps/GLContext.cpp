#include "GLContext.h"

GLContext::GLContext() : p_window(nullptr)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLContext::~GLContext()
{
    CloseWindow();
    glfwTerminate();
}

void GLContext::SetWinSize(int width, int height)
{
    win_width = width;
    win_height = height;
}

void GLContext::SetWinTitle(std::string title)
{
    if (!title.empty())
        win_title = title;
}

bool GLContext::GenGLWindow()
{
    CloseWindow();
    p_window = glfwCreateWindow(win_width, win_height, win_title.c_str(), NULL, NULL);
    if (p_window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        return false;
    }
    return true;
}

bool GLContext::ActivateWindow()
{
    if (!p_window)
        return false;
    glfwMakeContextCurrent(p_window);
    glfwShowWindow(p_window);
    glfwFocusWindow(p_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

bool GLContext::DoesWindowAboutToClose()
{
    return p_window ? glfwWindowShouldClose(p_window) : true;
}

const GLFWwindow* GLContext::GetWinPtr() const
{
    return p_window;
}

void GLContext::SetFlexibleView()
{
    if (p_window)
        glfwSetFramebufferSizeCallback(p_window, framebuffer_size_callback);
}

void GLContext::EnableInputControlWindowClosure()
{
    if (p_window && glfwGetKey(p_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(p_window, GLFW_TRUE);
}

void GLContext::SwapBuffers()
{
    if (p_window)
        glfwSwapBuffers(p_window);
}

void GLContext::PollEvents()
{
    glfwPollEvents();
}

void GLContext::CloseWindow()
{
    if (p_window)
    {
        glfwDestroyWindow(p_window);
        p_window = nullptr;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}