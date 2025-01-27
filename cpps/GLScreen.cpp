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

std::shared_ptr<GLBuffer> GLScreen::GetFrameBuffers(PixelShape shape_type, CompressedFrame&& a_frame)
{
    //static std::size_t last_frame_hash;
    //std::size_t cur_frame_hash = a_frame.GetPixelsHash();

    static std::shared_ptr<GLBuffer> last_frame_buffer;
    std::shared_ptr<GLBuffer> sptr_shape_buffer = std::make_shared<GLBuffer>();

    std::vector<float> frame_vertices;
    std::vector<unsigned int> frame_indices;

    // �����������ƫ����
    size_t vertex_offset = 0;

    std::vector<CubePixel> cubes;
    // std::vector<BallPixel> balls;
    // More shape more vectors

    switch (shape_type)
    {
    case CUBE:
        //if (last_frame_hash == cur_frame_hash)
        //{
        //    last_frame_hash = cur_frame_hash;
        //    return last_frame_buffer;
        //}
        //last_frame_hash = cur_frame_hash;

        for (const auto& pixel : a_frame.GetFrames())
            cubes.emplace_back(pixel);

        // ʹ�� ranges::for_each ����ڲ�� for ѭ��
        std::ranges::for_each(cubes, [&](auto& cube)
        {
            // ��ȡ��ǰ������Ķ�������
            auto vertices = cube.GetVertices();
            frame_vertices.insert(frame_vertices.end(), vertices.begin(), vertices.end());

            // ʹ�� ranges::transform ����������ֵ
            auto indices = cube.GetIndices();
            std::ranges::transform(indices, std::back_inserter(frame_indices),
                [&vertex_offset](unsigned int index) { return index + vertex_offset; });

            // ���¶���ƫ����
            vertex_offset += vertices.size() / cube.GetVerticesLength(); // ÿ�������� 12 ������
        });
        sptr_shape_buffer->SetVBOData(frame_vertices);
        sptr_shape_buffer->AllocateVBOMemo(0, 3, 12 * sizeof(float), 0);                // λ��
        sptr_shape_buffer->AllocateVBOMemo(1, 4, 12 * sizeof(float), 3 * sizeof(float)); // ��ɫ
        sptr_shape_buffer->AllocateVBOMemo(2, 3, 12 * sizeof(float), 7 * sizeof(float)); // ����
        sptr_shape_buffer->AllocateVBOMemo(3, 2, 12 * sizeof(float), 10 * sizeof(float)); // ��������
        sptr_shape_buffer->SetEBOData(frame_indices);
        sptr_shape_buffer->SetEBODataSize(frame_indices.size());
        sptr_shape_buffer->FinishInitialization();
        break;
    default:
        break;
    }
    last_frame_buffer = sptr_shape_buffer;
    return sptr_shape_buffer;
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

            // �ṩ���̿�����Ұ
            changeCameraStatusByKeyBoardInput(*gl_context, experence_shader, *gl_camera);

            // �����ɫ����Ȼ�����
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // �����ƶ��ӳ�
            gl_camera->UpdateSpeedByDeltaTime();
            gl_camera->UpdateFOV();
            view = gl_camera->GetLookUp();
            auto camera_pos = gl_camera->GetCameraPos();
            float camera_distance = glm::length(camera_pos);
            if (perspective_type)
                projection = glm::ortho(0.0f, (float)width / (camera_distance), 0.0f, (float)height / (camera_distance), 0.0f, camera_distance);
            else
                projection = glm::perspective(glm::radians(gl_camera->GetFOV()), (float)width / height, 0.1f, 100.0f);

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

            // �����������������¼�
            gl_context->SwapBuffers();
            gl_context->PollEvents();
        }
    });
    render_thread.join();
}

void GLScreen::RealTimeRendering(std::shared_ptr<IFilmStorage> sp_storage)
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
    uint64_t current_frame = 0;                // ��ǰ֡����

    std::shared_ptr<GLBuffer> frame_buffer = nullptr;

    InputHandler input_handler(const_cast<GLFWwindow*>(gl_context->GetWinPtr()));

    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::duration<double, std::milli>;

    while (!gl_context->DoesWindowAboutToClose())
    {
        auto loop_start_time = Clock::now();
        gl_context->EnableInputControlWindowClosure();

        // �ṩ���̿�����Ұ
        changeCameraStatusByKeyBoardInput(*gl_context, experence_shader, *gl_camera);

        // �����ɫ����Ȼ�����
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // �����ƶ��ӳ�
        gl_camera->UpdateSpeedByDeltaTime();
        gl_camera->UpdateFOV();
        view = gl_camera->GetLookUp();
        auto camera_pos = gl_camera->GetCameraPos();
        float camera_distance = glm::length(camera_pos);
        if (perspective_type)
            projection = glm::ortho(0.0f, (float)width / (camera_distance), 0.0f, (float)height / (camera_distance), 0.0f, camera_distance);
        else
            projection = glm::perspective(glm::radians(gl_camera->GetFOV()), (float)width / height, 0.1f, 100.0f);

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

        // ģ��Ӷ���ȡ��֡
        auto _f = sp_storage->Fetch();
        if (_f.GetFrames().empty())
            continue;
        frame_buffer = GetFrameBuffers(PixelShape::CUBE, std::move(_f));
        if (frame_buffer == nullptr)
            continue;
        current_frame++;

        // ģ�������Ϣ
        OpInfo op_info;
        input_handler.Update(op_info);
        if (op_info.activated)
        {
            op_info.op_frame_id = current_frame;
            concurrency_opinfo_queue.AddQuestToQueue(std::make_unique<OpInfo>(std::move(op_info)));
        }

        // ���Ƶ�ǰ֡
        frame_buffer->EnableVAO();
        glDrawElements(GL_TRIANGLES, frame_buffer->GetEBODataSize(), GL_UNSIGNED_INT, 0);
        frame_buffer->DisableVAO();

        // �����������������¼�
        gl_context->SwapBuffers();
        gl_context->PollEvents();

        auto loop_end_time = Clock::now();
        Duration loop_duration = loop_end_time - loop_start_time;
        //std::cout << "Render: " << loop_duration.count() << std::endl;
        //std::cout << "Current Frame ID: " << current_frame << std::endl;
    }
    // �˳�ʱ���ñ�־
    is_running = false;

    // ���ʣ��δ���͵Ĳ�����Ϣ
    concurrency_opinfo_queue.ClearQueue();
}

void GLScreen::TryGettingOpInfo(OpInfo& op_info)
{
    if (concurrency_opinfo_queue.Empty())
        return;
    op_info = *(concurrency_opinfo_queue.GetQuestFromQueue());
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

InputHandler::InputHandler(GLFWwindow* main_window)
{
    window = main_window;
    keyMap = {
        {GLFW_KEY_UP, "UP"}, {GLFW_KEY_DOWN, "DOWN"}, {GLFW_KEY_LEFT, "LEFT"}, {GLFW_KEY_RIGHT, "RIGHT"},
        {GLFW_KEY_SPACE, "SPACE"}, {GLFW_KEY_LEFT_SHIFT, "SHIFT"}
    };
}

void InputHandler::Update(OpInfo& op_info)
{
    HandleKeyboardInput(op_info);
    //HandleMouseInput(op_info);
}

void InputHandler::HandleKeyboardInput(OpInfo& op_info)
{
    std::string keyboardInfo;
    for (const auto& key : keyMap)
    {
        if (glfwGetKey(window, key.first) == GLFW_PRESS)
        {
            keyboardInfo = key.second;
        }
    }

    if (!keyboardInfo.empty())
    {
        op_info.activated = true;
        op_info.keyboard_info = keyboardInfo;
    }
}

void InputHandler::HandleMouseInput(OpInfo& op_info)
{
    static double lastX = 0, lastY = 0;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    std::string MouseInfo;
    if (xpos != lastX || ypos != lastY)
    {
        MouseInfo = std::to_string(xpos - lastX) + " " + std::to_string(ypos - lastY);
        lastX = xpos;
        lastY = ypos;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        MouseInfo = "Left mouse button pressed";
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        MouseInfo = "Right mouse button pressed";
    }

    if (!MouseInfo.empty())
    {
        op_info.activated = true;
        op_info.mouse_info = MouseInfo;
    }
}
