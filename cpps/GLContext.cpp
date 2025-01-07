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
    glfwTerminate();  // ��������ʱȷ����ֹGLFW
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
    // ֻ��һ����ǰ���������ܱ������ʹ��,��ȷ���κκ�����OpenGL������������������
    glfwMakeContextCurrent(p_window);
    // GLAD��ʼ����Ҫ������OpenGL�����ģ���OpenGL���������ڵ���glfwMakeContextCurrent����֮��Ŵ�����
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
    // �ڴ�������֮����Ⱦѭ����ʼ��֮ǰע����Щ�ص�����
    glfwSetFramebufferSizeCallback(p_window, framebuffer_size_callback);
}

void GLContext::EnableInputControlWindowClosure()
{
    if (glfwGetKey(p_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        // ������close flagΪtrue����ʹ��glfwWindowShouldClose����trueֵ
        glfwSetWindowShouldClose(p_window, GLFW_TRUE);
    }
}

void GLContext::SwapBuffers()
{
    // ����˫����
    glfwSwapBuffers(p_window);
}

void GLContext::PollEvents()
{
    // ������ѯ����Ƿ��������¼�
    glfwPollEvents();
}

void framebuffer_size_callback(GLFWwindow* p_window, int width, int height)
{
    // �� NDC �Ŵ���Ļ
    // ǰ���������Ǵ������½�����
    glViewport(0, 0, width, height);
}
