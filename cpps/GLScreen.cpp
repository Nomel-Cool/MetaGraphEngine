#include "GLScreen.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
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

void GLScreen::InitPerspectiveType(bool view_type)
{
    perspective_type = view_type;
}

void GLScreen::SetViewLock(bool is_view_lock)
{
    is_lock = is_view_lock;
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

std::vector<std::shared_ptr<GLBuffer>> GLScreen::GetFrameBuffers(PixelShape shape_type)
{
    std::vector<std::shared_ptr<GLBuffer>> frame_buffers;

    // ʹ�� ranges::iota ������� for ѭ��
    for (int i : std::views::iota(1, static_cast<int>(pixel_map.size())))
    {
        std::shared_ptr<GLBuffer> sptr_shape_buffer = std::make_shared<GLBuffer>();
        std::vector<float> frame_vertices;
        std::vector<unsigned int> frame_indices;

        // �����������ƫ����
        size_t vertex_offset = 0;

        // ʹ�� ranges::for_each ����ڲ�� for ѭ��
        std::ranges::for_each(pixel_map[i], [&](auto& pixel_shape) {
            // ��ȡ��ǰ������Ķ�������
            auto vertices = pixel_shape.GetVertices();
            frame_vertices.insert(frame_vertices.end(), vertices.begin(), vertices.end());

            // ʹ�� ranges::transform ����������ֵ
            auto indices = pixel_shape.GetIndices();
            std::ranges::transform(indices, std::back_inserter(frame_indices),
                [&vertex_offset](unsigned int index) { return index + vertex_offset; });

            // ���¶���ƫ����
            vertex_offset += vertices.size() / pixel_shape.GetVerticesLength(); // ÿ�������� 12 ������
        });

        switch (shape_type)
        {
        case CUBE:
            sptr_shape_buffer->SetVBOData(frame_vertices);
            sptr_shape_buffer->AllocateVBOMemo(0, 3, 12 * sizeof(float), 0);                // λ��
            sptr_shape_buffer->AllocateVBOMemo(1, 4, 12 * sizeof(float), 3 * sizeof(float)); // ��ɫ
            sptr_shape_buffer->AllocateVBOMemo(2, 3, 12 * sizeof(float), 7 * sizeof(float)); // ����
            sptr_shape_buffer->AllocateVBOMemo(3, 2, 12 * sizeof(float), 10 * sizeof(float)); // ��������
            sptr_shape_buffer->SetEBOData(frame_indices);
            sptr_shape_buffer->SetEBODataSize(frame_indices.size());
            sptr_shape_buffer->FinishInitialization();
            frame_buffers.emplace_back(sptr_shape_buffer);
            break;
        default:
            break;
        }
    }

    return frame_buffers;
}

std::vector<std::shared_ptr<GLBuffer>> GLScreen::GetFrameBuffers(PixelShape shape_type, CompressedFrame&& a_frame)
{
    static std::size_t last_frame_hash;
    std::size_t cur_frame_hash = a_frame.GetPixelsHash();

    static std::vector<std::shared_ptr<GLBuffer>> last_frame_buffers;
    std::vector<std::shared_ptr<GLBuffer>> frame_buffers;
    std::vector<CubePixel> cubes;
    // std::vector<BallPixel> balls;
    switch (shape_type)
    {
    case CUBE:
        if (last_frame_hash == cur_frame_hash)
        {
            last_frame_hash = cur_frame_hash;
            return last_frame_buffers;
        }
        last_frame_hash = cur_frame_hash;
        for (const auto& pixel : a_frame.GetFrames())
            cubes.emplace_back(pixel);
        // ʹ�� ranges::iota ������� for ѭ��
        for (int i : std::views::iota(0, static_cast<int>(cubes.size())))
        {
            std::shared_ptr<GLBuffer> sptr_shape_buffer = std::make_shared<GLBuffer>();
            std::vector<float> frame_vertices;
            std::vector<unsigned int> frame_indices;

            // �����������ƫ����
            size_t vertex_offset = 0;

            // ʹ�� ranges::for_each ����ڲ�� for ѭ��
            std::ranges::for_each(cubes, [&](auto& pixel_shape) 
            {
                // ��ȡ��ǰ������Ķ�������
                auto vertices = pixel_shape.GetVertices();
                frame_vertices.insert(frame_vertices.end(), vertices.begin(), vertices.end());

                // ʹ�� ranges::transform ����������ֵ
                auto indices = pixel_shape.GetIndices();
                std::ranges::transform(indices, std::back_inserter(frame_indices),
                    [&vertex_offset](unsigned int index) { return index + vertex_offset; });

                // ���¶���ƫ����
                vertex_offset += vertices.size() / pixel_shape.GetVerticesLength(); // ÿ�������� 12 ������
            });
            sptr_shape_buffer->SetVBOData(frame_vertices);
            sptr_shape_buffer->AllocateVBOMemo(0, 3, 12 * sizeof(float), 0);                // λ��
            sptr_shape_buffer->AllocateVBOMemo(1, 4, 12 * sizeof(float), 3 * sizeof(float)); // ��ɫ
            sptr_shape_buffer->AllocateVBOMemo(2, 3, 12 * sizeof(float), 7 * sizeof(float)); // ����
            sptr_shape_buffer->AllocateVBOMemo(3, 2, 12 * sizeof(float), 10 * sizeof(float)); // ��������
            sptr_shape_buffer->SetEBOData(frame_indices);
            sptr_shape_buffer->SetEBODataSize(frame_indices.size());
            sptr_shape_buffer->FinishInitialization();
            frame_buffers.emplace_back(sptr_shape_buffer);
        }
        break;
    default:
        break;
    }
    last_frame_buffers = frame_buffers;
    return frame_buffers;
}

void GLScreen::Rendering()
{
    render_thread = std::thread([&]() {
        glm::mat4 model = glm::mat4(1.0f), view = glm::mat4(1.0f), projection = glm::mat4(1.0f);

        gl_context->SetWinSize(width, height);
        gl_context->SetWinTitle("Graphs Stage");
        gl_context->GenGLWindow();
        gl_context->SetFlexibleView();
        gl_context->ActivateWindow();
        glfwSwapInterval(1); // ���ô�ֱͬ�� 60FPS
        glfwSetInputMode(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (!is_lock) // ����Ұ������������
        {
            glfwSetCursorPosCallback(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), mouse_callback);
            glfwSetScrollCallback(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), scroll_callback);        // ���ֿ�����Ұ
            glfwSetWindowUserPointer(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), &(*(gl_camera.get())));
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);

        GLShader experence_shader("./shaders_codes/vertex/vetex_with_color.vs",
            "./shaders_codes/fragment/ExperienceColor.fs");

        // ��ʼ���ƹ�λ�ø������
        // ������ܻ�����ǿ��                           // ��Դ���价�����ǿ��
        glm::vec3 ambientObject(1.0f, 0.5f, 0.31f), ambientLight(0.2f, 0.2f, 0.2f);
        // ��������������ǿ��                       // ��Դ������������ǿ��     
        glm::vec3 diffObject(1.0f, 0.5f, 0.31f), diffLight(0.5f, 0.5f, 0.5f);
        // ������ܾ��淴���ǿ��                     // ��Դ���侵�淴����ǿ��
        glm::vec3 specObject(0.5f, 0.5f, 0.5f), specLight(1.0f, 1.0f, 1.0f);
        // ���ʷ����
        float shininess = 64.0f;

        auto frame_buffers = GetFrameBuffers(PixelShape::CUBE);

        // ֡�ʿ���
        const double frame_duration = (float)1.0f / FPS; // 120 FPS
        double last_frame_time = glfwGetTime();   // ��һ֡��ʱ��
        size_t current_frame = 0;                // ��ǰ֡����

        while (!gl_context->DoesWindowAboutToClose())
        {
            gl_context->EnableInputControlWindowClosure();

            // �����ɫ����Ȼ�����
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // �����ƶ��ӳ�
            gl_camera->UpdateSpeedByDeltaTime();
            gl_camera->UpdateFOV();
            view = gl_camera->GetLookUp();
            float orthoSize = std::min(width, height) / 2.0f; // ����ͶӰ�İ�����
            float aspectRatio = (float)width / height; // ���ڵĿ�߱�
            if (perspective_type)
                projection = glm::ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio, -orthoSize, orthoSize, -100.0f, 100.0f);
            else
                projection = glm::perspective(glm::radians(gl_camera->GetFOV()), aspectRatio, 0.1f, 100.0f);

            // ��ȡ��ǰʱ��
            double current_time = glfwGetTime();
            double delta_time = current_time - last_frame_time;

            // ����ﵽ֡���ʱ�䣬�л�����һ֡
            if (delta_time >= frame_duration)
            {
                last_frame_time = current_time;

                // ��Ⱦ��ǰ֡
                auto& buffer = frame_buffers[current_frame];

                // ���·��߾���
                glm::mat3 normModelMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

                // ������ɫ��
                experence_shader.Use();
                experence_shader.SetMat3("normModelMatrix", normModelMatrix);
                experence_shader.SetMat4("view", view);
                experence_shader.SetMat4("projection", projection);
                experence_shader.SetVec3("viewPos", gl_camera->GetCameraPos());
                experence_shader.SetVec3("material.diffuse", diffObject);
                experence_shader.SetVec3("material.specular", specObject);
                experence_shader.SetFloat("material.shininess", shininess);
                experence_shader.SetVec3("light.position", gl_camera->GetCameraPos()); // �������Դ
                experence_shader.SetVec3("light.ambient", ambientLight);
                experence_shader.SetVec3("light.diffuse", diffLight);
                experence_shader.SetVec3("light.specular", specLight);

                // ���Ƶ�ǰ֡
                buffer->EnableVAO();
                glDrawElements(GL_TRIANGLES, buffer->GetEBODataSize(), GL_UNSIGNED_INT, 0);
                buffer->DisableVAO();

                // �л�����һ֡
                current_frame = (current_frame + 1) % frame_buffers.size();
            }

            // �ṩ���̿�����Ұ
            changeCameraStatusByKeyBoardInput(*gl_context, experence_shader, *gl_camera);

            // �����������������¼�
            gl_context->SwapBuffers();
            gl_context->PollEvents();
        }
    });
    render_thread.join();
}

void GLScreen::RealTimeRendering(PhotoGrapher& photo_grapher)
{
    glm::mat4 model = glm::mat4(1.0f), view = glm::mat4(1.0f), projection = glm::mat4(1.0f);

    gl_context->SetWinSize(width, height);
    gl_context->SetWinTitle("Graphs Stage");
    gl_context->GenGLWindow();
    gl_context->SetFlexibleView();
    gl_context->ActivateWindow();
    glfwSwapInterval(1); // ���ô�ֱͬ�� 60FPS
    glfwSetInputMode(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!is_lock) // ����Ұ������������
    {
        glfwSetCursorPosCallback(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), mouse_callback);
        glfwSetScrollCallback(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), scroll_callback);        // ���ֿ�����Ұ
        glfwSetWindowUserPointer(const_cast<GLFWwindow*>(gl_context->GetWinPtr()), &(*(gl_camera.get())));
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    GLShader experence_shader("./shaders_codes/vertex/vetex_with_color.vs",
        "./shaders_codes/fragment/ExperienceColor.fs");

    // ��ʼ���ƹ�λ�ø������
    // ������ܻ�����ǿ��                           // ��Դ���价�����ǿ��
    glm::vec3 ambientObject(1.0f, 0.5f, 0.31f), ambientLight(0.2f, 0.2f, 0.2f);
    // ��������������ǿ��                       // ��Դ������������ǿ��     
    glm::vec3 diffObject(1.0f, 0.5f, 0.31f), diffLight(0.5f, 0.5f, 0.5f);
    // ������ܾ��淴���ǿ��                     // ��Դ���侵�淴����ǿ��
    glm::vec3 specObject(0.5f, 0.5f, 0.5f), specLight(1.0f, 1.0f, 1.0f);
    // ���ʷ����
    float shininess = 64.0f;

    // ֡�ʿ���
    const double frame_duration = (float)1.0f / FPS; // 120 FPS
    double last_frame_time = glfwGetTime();   // ��һ֡��ʱ��
    size_t current_frame = 0;                // ��ǰ֡����

    std::vector<std::shared_ptr<GLBuffer>> frame_buffers;
    OpInfo op_info;

    while (!gl_context->DoesWindowAboutToClose())
    {
        gl_context->EnableInputControlWindowClosure();

        // �����ɫ����Ȼ�����
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ģ�������Ϣ
        //op_info.op_name = op_info.op_name + std::to_string(last_frame_time);
        //concurrency_opinfo_queue.AddQuestToQueue(std::make_unique<OpInfo>(std::move(op_info)));

        // ģ��Ӷ���ȡ��֡
        auto _f = photo_grapher.TryGettingFrame();
        if (_f.GetFrames().empty())
            continue;
        frame_buffers = GetFrameBuffers(PixelShape::CUBE, std::move(_f));
        if (frame_buffers.empty())
            continue;

        // �����ƶ��ӳ�
        gl_camera->UpdateSpeedByDeltaTime();
        gl_camera->UpdateFOV();
        view = gl_camera->GetLookUp();
        float orthoSize = std::min(width, height) / 2.0f; // ����ͶӰ�İ�����
        float aspectRatio = (float)width / height; // ���ڵĿ�߱�
        if (perspective_type)
            projection = glm::ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio, -orthoSize, orthoSize, -100.0f, 100.0f);
        else
            projection = glm::perspective(glm::radians(gl_camera->GetFOV()), aspectRatio, 0.1f, 100.0f);

        // ��ȡ��ǰʱ��
        double current_time = glfwGetTime();
        double delta_time = current_time - last_frame_time;

        // ����ﵽ֡���ʱ�䣬�л�����һ֡
        if (delta_time >= frame_duration)
        {
            last_frame_time = current_time;

            // ��Ⱦ��ǰ֡
            auto& buffer = frame_buffers[current_frame];

            // ���·��߾���
            glm::mat3 normModelMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

            // ������ɫ��
            experence_shader.Use();
            experence_shader.SetMat3("normModelMatrix", normModelMatrix);
            experence_shader.SetMat4("view", view);
            experence_shader.SetMat4("projection", projection);
            experence_shader.SetVec3("viewPos", gl_camera->GetCameraPos());
            experence_shader.SetVec3("material.diffuse", diffObject);
            experence_shader.SetVec3("material.specular", specObject);
            experence_shader.SetFloat("material.shininess", shininess);
            experence_shader.SetVec3("light.position", gl_camera->GetCameraPos()); // �������Դ
            experence_shader.SetVec3("light.ambient", ambientLight);
            experence_shader.SetVec3("light.diffuse", diffLight);
            experence_shader.SetVec3("light.specular", specLight);

            // ���Ƶ�ǰ֡
            buffer->EnableVAO();
            glDrawElements(GL_TRIANGLES, buffer->GetEBODataSize(), GL_UNSIGNED_INT, 0);
            buffer->DisableVAO();

            // �л�����һ֡
            current_frame = (current_frame + 1) % frame_buffers.size();
        }

        // �ṩ���̿�����Ұ
        changeCameraStatusByKeyBoardInput(*gl_context, experence_shader, *gl_camera);

        // �����������������¼�
        gl_context->SwapBuffers();
        gl_context->PollEvents();
    }
}

OpInfo GLScreen::TryGettingOpInfo()
{
    OpInfo nothing;
    nothing.op_name = "nothing...";
    if (concurrency_opinfo_queue.Empty())
        return nothing;
    auto up_op_info = concurrency_opinfo_queue.GetQuestFromQueue();
    return *up_op_info;
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    static GLCamera* camera = reinterpret_cast<GLCamera*>(glfwGetWindowUserPointer(window));

    if (camera)
    {
        // ���ݹ����������Ŀ�� FOV
        float currentFOV = camera->GetFOV();
        float targetFOV = currentFOV - static_cast<float>(yoffset); // yoffset �ǹ��ֵĴ�ֱƫ����
        camera->SetTargetFOV(targetFOV);
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