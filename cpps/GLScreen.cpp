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

        // ��ʼ���ƹ�
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        // ������ܻ�����ǿ��                           // ��Դ���价�����ǿ��
        glm::vec3 ambientObject(1.0f, 0.5f, 0.31f), ambientLight(0.2f, 0.2f, 0.2f);
        // ��������������ǿ��                       // ��Դ������������ǿ��     
        glm::vec3 diffObject(1.0f, 0.5f, 0.31f), diffLight(0.5f, 0.5f, 0.5f);
        // ������ܾ��淴���ǿ��                     // ��Դ���侵�淴����ǿ��
        glm::vec3 specObject(0.5f, 0.5f, 0.5f), specLight(1.0f, 1.0f, 1.0f);
        // ���ʷ����
        float shininess = 64.0f;

        while (!gl_context->DoesWindowAboutToClose())
        {
            gl_context->EnableInputControlWindowClosure();

            // �����ɫ����Ȼ�����
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // �����ƶ��ӳ�
            gl_camera->UpdateSpeedByDeltaTime();
            view = gl_camera->GetLookUp();

            for (int i = 0; i < pixel_map.size(); ++i)
            {
                for (auto& cube_pixel : pixel_map[i])
                {
                    auto vertices = cube_pixel.GetVertices();
                    auto indices = cube_pixel.GetIndices();
                    model = cube_pixel.GetTransformMat();

                    // ����buffer
                    GLBuffer box_buffer;
                    box_buffer.SetVBOData(vertices);
                    box_buffer.AllocateVBOMemo(0, 3, 12 * sizeof(float), 0);
                    box_buffer.AllocateVBOMemo(1, 4, 12 * sizeof(float), 3 * sizeof(float));
                    box_buffer.AllocateVBOMemo(2, 3, 12 * sizeof(float), 7 * sizeof(float));
                    box_buffer.AllocateVBOMemo(3, 2, 12 * sizeof(float), 10 * sizeof(float));
                    box_buffer.SetEBOData(indices);
                    box_buffer.FinishInitialization();

                    // ���·��߾���
                    glm::mat3 normModelMatrix = glm::transpose(glm::inverse(glm::mat3(model))); // ��ȡƽ�Ʋ��֣���Ϊ��������ƽ��û���壬���߰ѷ���������η�����Ϊw=0
                    
                    // ������ɫ��
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
                    // ����
                    box_buffer.EnableVAO();
                    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
                    box_buffer.DisableVAO();
                }
            }

            // �ṩ���̿�����Ұ
            changeCameraStatusByKeyBoardInput(*gl_context, experence_shader, *gl_camera);

            // �����������������¼�
            gl_context->SwapBuffers();
            gl_context->PollEvents();
        }
    });

}

void changeCameraStatusByKeyBoardInput(GLContext& context_manager, GLShader& shader_manager, GLCamera& camera_manager)
{
    static bool upPressed = false;
    static bool downPressed = false;
    if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_W) == GLFW_PRESS) // �� -z �ƶ���ǰ��
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::FORWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_S) == GLFW_PRESS) // �� +z �ƶ�����
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::BACKWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_A) == GLFW_PRESS) // �� -x �ƶ�����
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::LEFTWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_D) == GLFW_PRESS) // �� +x �ƶ����ң�
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::RIGHTWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_Q) == GLFW_PRESS) // �� +y �ƶ����ϣ�
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::UPWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_E) == GLFW_PRESS) // �� -y �ƶ����£�
    {
        camera_manager.CameraMove(CAMERA_MOVE_TYPE::DOWNWARD, 1.0f);
        auto view_mat = camera_manager.GetLookUp();
        shader_manager.Use();
        shader_manager.SetMat4("view", view_mat);
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_I) == GLFW_PRESS) // �� x �� ��ʱ��
    {
        if (!upPressed) // ����Ƿ��Ѿ�����
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDX, 10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // ���ð���״̬Ϊ�Ѱ���
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_K) == GLFW_PRESS) // �� x �� ˳ʱ��
    {
        if (!upPressed) // ����Ƿ��Ѿ�����
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDX, -10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // ���ð���״̬Ϊ�Ѱ���
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_J) == GLFW_PRESS) // �� z �� ��ʱ��
    {
        if (!upPressed) // ����Ƿ��Ѿ�����
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDZ, 10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // ���ð���״̬Ϊ�Ѱ���
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_L) == GLFW_PRESS)// �� z �� ˳ʱ��
    {
        if (!upPressed) // ����Ƿ��Ѿ�����
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDZ, -10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // ���ð���״̬Ϊ�Ѱ���
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_U) == GLFW_PRESS) // �� y �� ��ʱ��
    {
        if (!upPressed) // ����Ƿ��Ѿ�����
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDY, 10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // ���ð���״̬Ϊ�Ѱ���
        }
    }
    else if (glfwGetKey(const_cast<GLFWwindow*>(context_manager.GetWinPtr()), GLFW_KEY_O) == GLFW_PRESS)// �� y �� ˳ʱ��
    {
        if (!upPressed) // ����Ƿ��Ѿ�����
        {
            camera_manager.CameraSpin(CAMERA_SPIN_TYPE::SPINAROUNDY, -10.0f);
            auto view_mat = camera_manager.GetLookUp();
            shader_manager.Use();
            shader_manager.SetMat4("view", view_mat);
            upPressed = true; // ���ð���״̬Ϊ�Ѱ���
        }
    }
    else
    {
        upPressed = false; // ���ð���״̬
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static GLCamera* camera = reinterpret_cast<GLCamera*>(glfwGetWindowUserPointer(window));
    static GLShader* shader_manager = reinterpret_cast<GLShader*>(glfwGetWindowUserPointer(window));
    static float lastX = 400; // ��ʼ�����λ��
    static float lastY = 300;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // ע�⣺y �����Ƿ���
    lastX = xpos;
    lastY = ypos;

    // ���� CameraSpin ���������ת
    camera->CameraSpin(xoffset, yoffset);
}