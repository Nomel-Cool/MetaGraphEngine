#include "GLScreen.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void changeCameraStatusByKeyBoardInput(GLContext& context_manager, GLShader& shader_manager, GLCamera& camera_manager);

GLScreen::~GLScreen()
{
    if(render_thread.joinable())
        render_thread.join();
}

void GLScreen::InitScreen(int screen_width, int screen_height)
{
    width = screen_width;
    height = screen_height;
}

void GLScreen::InitCamara(glm::vec3 camera_pos)
{
    gl_camera = std::make_shared<GLCamera>(camera_pos);
}

void GLScreen::SetVerticesData(const std::vector<CubePixel>& cubes)
{
    for (const auto& cube : cubes)
    {
        if (pixel_map.find(cube.cur_frame_id) == pixel_map.end())
        {
            pixel_map.insert(std::make_pair(cube.cur_frame_id, std::vector<CubePixel>()));
            pixel_map[cube.cur_frame_id].emplace_back(cube);
        }
        else
            pixel_map[cube.cur_frame_id].emplace_back(cube);
    }
}

void GLScreen::Rendering()
{
    render_thread = std::thread([&]() {
        glm::mat4 model = glm::mat4(1.0f), view = glm::mat4(1.0f), projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

        gl_context->SetWinSize(width, height);
        gl_context->SetWinTitle("Graphs Stage");
        gl_context->GenGLWindow();
        gl_context->SetFlexibleView();
        gl_context->ActivateWindow();
        glfwSetInputMode(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), mouse_callback);
        glfwSetWindowUserPointer(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), &(*(gl_camera.get())));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);

        GLShader experence_shader("./shaders_codes/vertex/vetex_with_color.vs",
            "./shaders_codes/fragment/ExperienceColor.fs");

        // 初始化灯光
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        // 物体接受环境光强度                           // 光源发射环境光的强度
        glm::vec3 ambientObject(1.0f, 0.5f, 0.31f), ambientLight(0.2f, 0.2f, 0.2f);
        // 物体接受漫反射光强度                       // 光源发射漫反射光的强度     
        glm::vec3 diffObject(1.0f, 0.5f, 0.31f), diffLight(0.5f, 0.5f, 0.5f);
        // 物体接受镜面反射光强度                     // 光源发射镜面反射光的强度
        glm::vec3 specObject(0.5f, 0.5f, 0.5f), specLight(1.0f, 1.0f, 1.0f);
        // 材质反光度
        float shininess = 64.0f;

        while (!gl_context->DoesWindowAboutToClose())
        {
            gl_context->EnableInputControlWindowClosure();

            // 清除颜色和深度缓冲区
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 修正移动延迟
            gl_camera->UpdateSpeedByDeltaTime();
            view = gl_camera->GetLookUp();

            for (int i = 0; i < pixel_map.size(); ++i)
            {
                for (auto& cube_pixel : pixel_map[i])
                {
                    auto vertices = cube_pixel.GetVertices();
                    auto indices = cube_pixel.GetIndices();
                    model = cube_pixel.GetTransformMat();

                    // 设置buffer
                    GLBuffer box_buffer;
                    box_buffer.SetVBOData(vertices);
                    box_buffer.AllocateVBOMemo(0, 3, 12 * sizeof(float), 0);
                    box_buffer.AllocateVBOMemo(1, 4, 12 * sizeof(float), 3 * sizeof(float));
                    box_buffer.AllocateVBOMemo(2, 3, 12 * sizeof(float), 7 * sizeof(float));
                    box_buffer.AllocateVBOMemo(3, 2, 12 * sizeof(float), 10 * sizeof(float));
                    box_buffer.SetEBOData(indices);
                    box_buffer.FinishInitialization();

                    // 更新法线矩阵
                    glm::mat3 normModelMatrix = glm::transpose(glm::inverse(glm::mat3(model))); // 切取平移部分，因为方向向量平移没意义，或者把方向向量齐次分量设为w=0
                    
                    // 设置着色器
                    experence_shader.Use();
                    experence_shader.SetMat3("normModelMatrix", normModelMatrix);
                    experence_shader.SetMat4("model", model);
                    experence_shader.SetMat4("view", view);
                    experence_shader.SetMat4("projection", projection);
                    experence_shader.SetVec3("viewPos", gl_camera->GetCameraPos());
                    experence_shader.SetVec3("material.diffuse", diffObject);
                    experence_shader.SetVec3("material.specular", specObject);
                    experence_shader.SetFloat("material.shininess", shininess);
                    experence_shader.SetVec3("light.position", lightPos);
                    experence_shader.SetVec3("light.ambient", ambientLight);
                    experence_shader.SetVec3("light.diffuse", diffLight);
                    experence_shader.SetVec3("light.specular", specLight);
                    // 绘制
                    box_buffer.EnableVAO();
                    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
                    box_buffer.DisableVAO();
                }
            }

            // 提供键盘控制视野
            changeCameraStatusByKeyBoardInput(*gl_context, experence_shader, *gl_camera);

            // 交换缓冲区并处理事件
            gl_context->SwapBuffers();
            gl_context->PollEvents();
        }
    });

}

void changeCameraStatusByKeyBoardInput(GLContext& context_manager, GLShader& shader_manager, GLCamera& camera_manager)
{
    static bool upPressed = false;
    static bool downPressed = false;
    if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_W) == GLFW_PRESS) // 沿 -z 移动（前）
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::FORWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_S) == GLFW_PRESS) // 沿 +z 移动（后）
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::BACKWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_A) == GLFW_PRESS) // 沿 -x 移动（左）
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::LEFTWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_D) == GLFW_PRESS) // 沿 +x 移动（右）
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::RIGHTWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_Q) == GLFW_PRESS) // 沿 +y 移动（上）
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::UPWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_E) == GLFW_PRESS) // 沿 -y 移动（下）
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::DOWNWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_I) == GLFW_PRESS) // 绕 x 轴 逆时针
    {
        if (!upPressed) // 检查是否已经按下
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDX, 10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // 设置按键状态为已按下
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_K) == GLFW_PRESS) // 绕 x 轴 顺时针
    {
        if (!upPressed) // 检查是否已经按下
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDX, -10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // 设置按键状态为已按下
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_J) == GLFW_PRESS) // 绕 z 轴 逆时针
    {
        if (!upPressed) // 检查是否已经按下
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDZ, 10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // 设置按键状态为已按下
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_L) == GLFW_PRESS)// 绕 z 轴 顺时针
    {
        if (!upPressed) // 检查是否已经按下
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDZ, -10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // 设置按键状态为已按下
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_U) == GLFW_PRESS) // 绕 y 轴 逆时针
    {
        if (!upPressed) // 检查是否已经按下
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDY, 10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // 设置按键状态为已按下
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_O) == GLFW_PRESS)// 绕 y 轴 顺时针
    {
        if (!upPressed) // 检查是否已经按下
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDY, -10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // 设置按键状态为已按下
        }
    }
    else
    {
        upPressed = false; // 重置按键状态
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static GLCamera* camera = reinterpret_cast<GLCamera*>(glfwGetWindowUserPointer(window));
    static GLShader* shader_manager = reinterpret_cast<GLShader*>(glfwGetWindowUserPointer(window));
    static float lastX = 400; // 初始化鼠标位置
    static float lastY = 300;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意：y 方向是反的
    lastX = xpos;
    lastY = ypos;

    // 调用 CameraSpin 更新相机旋转
    camera->CameraSpin(xoffset, yoffset);
}