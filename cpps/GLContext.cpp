#include "GLContext.h"

GLContext::GLContext(): p_window(nullptr)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLContext::~GLContext()
{
    if (p_window) {
        glfwDestroyWindow(p_window);
    }
    glfwTerminate();  // 在类析构时确保终止GLFW
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
    p_window = glfwCreateWindow(win_width, win_height, win_title.c_str(), NULL, NULL);
    if (p_window == nullptr)
    {
        std::cout << "Failed to create GLFW p_window" << std::endl;
        glfwTerminate();
        return false;
    }
    return true;
}

bool GLContext::ActivateWindow()
{
    // 只有一个当前的上下文能被激活和使用,它确保任何后续的OpenGL命令会作用于这个窗口
    glfwMakeContextCurrent(p_window);
    // GLAD初始化需要依赖于OpenGL上下文，而OpenGL上下文是在调用glfwMakeContextCurrent激活之后才创建的
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

bool GLContext::DoesWindowAboutToClose()
{
    return glfwWindowShouldClose(p_window);
}

const GLFWwindow* GLContext::GetWinPtr() const
{
    return p_window;
}

void GLContext::SetFlexibleView()
{
    // 在创建窗口之后，渲染循环初始化之前注册这些回调函数
    glfwSetFramebufferSizeCallback(p_window, framebuffer_size_callback);
}

void GLContext::EnableInputControlWindowClosure()
{
    if (glfwGetKey(p_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        // 设置让close flag为true，这使得glfwWindowShouldClose返回true值
        glfwSetWindowShouldClose(p_window, GLFW_TRUE);
    }
}

void GLContext::SwapBuffers()
{
    // 设置双缓冲
    glfwSwapBuffers(p_window);
}

void GLContext::PollEvents()
{
    // 设置轮询检查是否有输入事件
    glfwPollEvents();
}

void framebuffer_size_callback(GLFWwindow* p_window, int width, int height)
{
    // 将 NDC 放大到屏幕
    // 前两个参数是窗口左下角坐标
    glViewport(0, 0, width, height);
}
