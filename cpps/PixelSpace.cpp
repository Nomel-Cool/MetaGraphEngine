#include "PixelSpace.h"

GraphStudio::GraphStudio(QObject* parent) : QObject(parent)
{
    sp_frame_id_manager = std::make_shared<FrameIDManager>();
    sp_frame_generate_interval_manager = std::make_shared<FrameGenerateIntervalManager>();
    sp_filmname_manager = std::make_shared<FilmNameManager>();

    auto factory = std::make_shared<GraphFactory>();
    auto cohandler_cache = std::make_shared<ThreadSafeGraphCache>();
    auto model_loader = std::make_unique<GraphModelLoader>(factory);
    auto corotine_manager = std::make_unique<CoroutineExecutor>(cohandler_cache);
    sp_graph_agency = std::make_shared<GraphAgency>(std::move(model_loader),cohandler_cache,std::move(corotine_manager));

    sp_hall = std::make_shared<Hall>(sp_frame_id_manager);
    sp_law = std::make_shared<Law>();
    sp_timer = std::make_shared<QTimer>(this);
    sp_gl_screen = std::make_shared<GLScreen>();
    sp_film_static_storage = std::make_shared<FilmNameMap>();
    sp_film_realtime_storage = std::make_shared<TaskModelFrameQueue>();
    sp_realtime_photographer = std::make_shared<RealTimePhotoGrapher>(sp_film_realtime_storage);
    sp_static_photographer = std::make_shared<StaticPhotoGrapher>(sp_film_static_storage);

    // 如果使用其它第三方XML解析库修改工程指针即可
    std::unique_ptr<shabby::IXMLDocumentFactory> tinyxml_factory = std::make_unique<TinyXMLDocumentFactory>();
    sp_file_manager = std::make_shared<FileManager>(std::move(tinyxml_factory));
}

void GraphStudio::InitWindow(int width, int height, float cameraX, float cameraY, float cameraZ, bool perspective_type, bool view_lock)
{
    sp_gl_screen->InitScreen(width, height);
    sp_gl_screen->InitCamara(glm::vec3(cameraX, cameraY, cameraZ));
    sp_gl_screen->InitPerspectiveType(perspective_type);
    sp_gl_screen->SetViewLock(view_lock);
}

void GraphStudio::SetFilmName(const QString& film_name)
{
    sp_filmname_manager->SetCurrentFilmName(film_name.toStdString());
}

void GraphStudio::RoleEmplacement(const QStringList& model_names)
{
    for (const QString& model_name : model_names)
    {
        pool.Enqueue(&GraphAgency::LoadGraphs, sp_graph_agency, model_name);
        // 或者调用 sp_graph_agency->LoadGraphs(model_name);
    }
}

void GraphStudio::Display(const QStringList& film_name_list)
{
    if (film_name_list.empty())
        return;
    // 遍历所有需要播放的帧数据名
    for (const auto& film_name : film_name_list)
    {
        std::vector<CubePixel> vec_cubes;
        auto compressed_pics = sp_film_static_storage->Fetch(film_name.toStdString());
        auto vec_pics = compressed_pics.GetFrames();
        for (const OnePixel& pic : vec_pics)
            vec_cubes.emplace_back(pic);
        sp_gl_screen->SetVerticesData(vec_cubes); // 按帧顺序设置顶点数据
    }

    // 创建窗口 并 执行渲染
    sp_gl_screen->Rendering();

}

void GraphStudio::Launch()
{
    try
    {
        // 每1/30秒为一个时间片，在此时间片内需要做：
        // 1.处理的主要对象：for (auto& model : sp_graph_agency->GetGraphs())
        // 2.取model的CurrentStatus中的点，执行stage[{x,y}]的rule，可能会修改model的所有信息
        // 3.由于每条线程负责一个图元，所以不必再开线程画图，每个图元都已经抵达了这里
        // 4.遍历stage压缩渲染点信息
        // 5.model执行resume
        if (sp_graph_agency->Empty())
        {
            std::cerr << "No actor..." << std::endl;
            return;
        }
        auto render_loop = [this]()
        {
            static bool running = true;
            if (!running)
                return;
            running = false;

            /**************** 暂定流程四件套 ***************/
            StandBy(); // 出牌定格
            Interact(); // 变更手牌
            UpdateGraphList(); // 收牌再来
            SnapShot(); // 储为快照
            TidyUp(); // 清理非渲染像素

            running = true;
        };
        connect(sp_timer.get(), &QTimer::timeout, render_loop);
        sp_timer->start(sp_frame_generate_interval_manager->GetFrameGenerationInterval()); // FPS ≈ 120
    }
    catch (const std::logic_error)
    {
        Stop();
    }
}

void GraphStudio::RealTimeRender()
{
    std::thread tmp1 = std::thread(
        [this]() {
        sp_gl_screen->RealTimeRendering(sp_film_realtime_storage);
        sp_gl_screen->is_running = false; // tmp1 退出时，通知 tmp2 退出
    });

    std::thread tmp2 = std::thread(
        [this]()
        {
            using Clock = std::chrono::high_resolution_clock;
            using Duration = std::chrono::duration<double, std::milli>;
            sp_realtime_photographer->SetUsageStatus(true);
            while (sp_gl_screen->is_running)
            {
                auto loop_start_time = Clock::now();
                if (!sp_gl_screen->is_running) break; // 如果 tmp1 退出，则退出循环

                ///**************** 暂定流程四件套 ***************/
                StandBy();          // 出牌定格
                RealTimeInteract(); // 根据操作实时变更手牌
                UpdateGraphList();  // 收牌再来
                TidyUp();           // 清理非渲染像素
                RealTimeSnapShot(); // 储为实时快照
                auto loop_end_time = Clock::now();
                Duration loop_duration = loop_end_time - loop_start_time;
                auto sleep_duration = std::chrono::milliseconds(sp_frame_generate_interval_manager->GetFrameGenerationInterval()) - loop_duration; // 限制为 58 FPS
                if (sleep_duration.count() > 0)
                    std::this_thread::sleep_for(sleep_duration);
                //std::cout << "Generate: " << sleep_duration.count() << std::endl;
                //std::cout << "Total Frame ID: " << sp_hall->GetCurrentFrameID() << std::endl;
            }
            sp_film_realtime_storage->ClearMemo();
        }
    );
    tmp1.join();
    tmp2.join();
}

void GraphStudio::Ceize()
{
    sp_timer->stop();
    disconnect(sp_timer.get(), &QTimer::timeout, nullptr, nullptr);
    sp_static_photographer->FilmDone(sp_filmname_manager->GetCurrentFilmName());
    sp_filmname_manager->ResetCurrentFilmName();
}

void GraphStudio::RoleDismiss()
{
    sp_graph_agency->CleanGraphCache();
}

void GraphStudio::Stop()
{
    sp_static_photographer->FilmDone(sp_filmname_manager->GetCurrentFilmName());
    if (sp_realtime_photographer->GetUsageStatus())
        return;
    if (!sp_graph_agency->Inspect())
        return;
    sp_timer->stop();
    disconnect(sp_timer.get(), &QTimer::timeout, nullptr, nullptr);
    emit filmTerminated(QString::fromStdString(sp_filmname_manager->GetCurrentFilmName()));
    sp_filmname_manager->ResetCurrentFilmName();
}

void GraphStudio::StandBy()
{
    auto& graph_list = sp_graph_agency->GetGraphs();
    for (std::size_t i = 0; i < graph_list.size(); ++i)
    {
        try
        {
            json current_status;
            auto str_cur_pos = graph_list[i]->GetValue().current_status;
            if (!sp_file_manager->TransStr2JsonObject(str_cur_pos, current_status))
            {
                std::cerr << "failed to parse json: " << str_cur_pos << std::endl;
                return;
            }
            // 根据协议，所有字段均为数组
            if (!current_status.is_array())
                return;
            std::vector<OnePixel> point_list;
            UpdatePixelsThroughAutomata(point_list, current_status);
            sp_hall->PingStage(point_list, i);
        }
        catch (std::logic_error)
        {
            Stop();
        }
    }
}

void GraphStudio::Interact()
{
    sp_law->AffectOn<Gravity>(this);
}

void GraphStudio::RealTimeInteract()
{
    OpInfo op_info;
    sp_gl_screen->TryGettingOpInfo(op_info);
    sp_law->AffectOn<Gravity>(this, op_info);
}

void GraphStudio::UpdateGraphList()
{
    try
    {
        const auto& activating_pixels = sp_hall->CollectStage();
        for (const auto& pixels : activating_pixels)
        {
            AutomataElements automata_param = GetAutomataInfoAt(pixels.first);
            // 此处可以手动更新当前渲染点的自动机信息
            // 我并没有做边界检测，可能是个空的json，而一旦读取空json就会抛出异常使得程序崩溃，所以请确保你的图元蓝图里确实具有该字段
            json initial_status, current_status, current_input, terminate_status;
            std::tie(initial_status, current_status, current_input, terminate_status) = automata_param;
            // 因为不清楚这个自动机有哪些字段，应当做函数指针传入来处理，这里先手动更新位置
            current_status.clear();
            UpdateAutomataThroughPixels(current_status, pixels.second);
            /*******************************************************************/
            automata_param = std::make_tuple(initial_status, current_status, current_input, terminate_status);
            SetAutomataInfoAt(pixels.first, automata_param);
        }
        sp_graph_agency->UpdateGraphs(); // 返回协程算法
    }
    catch (std::logic_error)
    {
        std::cerr << "The model is done, break out!!!" << std::endl;
        Stop();
    }
}

void GraphStudio::SnapShot()
{
    auto cur_id = sp_frame_id_manager->GetCurrentFrameID();
    for (auto& pixel : sp_hall->GetStage())
    {
        std::shared_ptr<OnePixel> specific_pixel = pixel.second;
        if (specific_pixel->cur_frame_id == cur_id && specific_pixel->render_flag)
            sp_static_photographer->Filming(*specific_pixel);
    }
    sp_frame_id_manager->NextFrame();
}

void GraphStudio::RealTimeSnapShot()
{
    auto cur_id = sp_frame_id_manager->GetCurrentFrameID();
    for (auto& pixel : sp_hall->GetStage())
    {
        std::shared_ptr<OnePixel> specific_pixel = pixel.second;
        if (specific_pixel->cur_frame_id == cur_id && specific_pixel->render_flag)
            sp_realtime_photographer->Filming(*specific_pixel);
    }
    sp_realtime_photographer->FilmDone();
    sp_frame_id_manager->NextFrame();
}

void GraphStudio::TidyUp()
{
    for (auto iter = sp_hall->GetStage().begin(); iter != sp_hall->GetStage().end();)
        if (iter->second == nullptr || iter->second->owners_info.empty())
            iter = sp_hall->DeleteElementAt(iter->first);
        else
            ++iter;
}

AutomataElements GraphStudio::GetAutomataInfoAt(std::size_t indice)
{
    json init_status, current_status, current_input, terminate_status;

    try
    {
        auto& graph_list = sp_graph_agency->GetGraphs();
        auto& graph_model = graph_list[indice]->GetValue();

        if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
            throw std::logic_error("Failed to parse init_status JSON");
        }
        if (!sp_file_manager->TransStr2JsonObject(graph_model.current_status, current_status))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
            throw std::logic_error("Failed to parse current_status JSON");
        }
        if (!sp_file_manager->TransStr2JsonObject(graph_model.current_input, current_input))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.current_input << std::endl;
            throw std::logic_error("Failed to parse current_input JSON");
        }
        if (!sp_file_manager->TransStr2JsonObject(graph_model.terminate_status, terminate_status))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.terminate_status << std::endl;
            throw std::logic_error("Failed to parse terminate_status JSON");
        }
    }
    catch (const std::logic_error& e)
    {
        //std::cerr << "Error: " << e.what() << std::endl;
    }
    return std::make_tuple(init_status, current_status, current_input, terminate_status);
}

void GraphStudio::SetAutomataInfoAt(std::size_t indice, const AutomataElements& automata_status)
{
    try
    {
        auto& graph_list = sp_graph_agency->GetGraphs();
        auto& graph_model = graph_list[indice]->GetValue();
        json init_status, current_status, current_input, terminate_status;
        std::tie(init_status, current_status, current_input, terminate_status) = automata_status;
        graph_model.init_status = init_status.dump();
        graph_model.current_status = current_status.dump();
        graph_model.current_input = current_input.dump();
        graph_model.terminate_status = terminate_status.dump();
    }
    catch (std::logic_error e)
    {
        //std::cerr << "The model is done, break out!!!" << std::endl;
        return;
    }
}

void GraphStudio::UpdatePixelsThroughAutomata(std::vector<OnePixel>& points, const json& status)
{
    for (const auto& point : status)
    {
        OnePixel ping_pixel;
        ping_pixel.x = point["x"];
        ping_pixel.y = point["y"];
        ping_pixel.z = point["z"];
        ping_pixel.r = point["r"];
        ping_pixel.g = point["g"];
        ping_pixel.b = point["b"];
        ping_pixel.a = point["a"];
        ping_pixel.tag = point["tag"];
        ping_pixel.block_size = point["blockSize"];
        ping_pixel.cur_frame_id = sp_frame_id_manager->GetCurrentFrameID();
        points.emplace_back(ping_pixel);
    }
}

void GraphStudio::UpdateAutomataThroughPixels(json& status, const std::vector<OnePixel>& points)
{
    for (const auto& activate_pixel : points)
    {
        status.push_back({
            { "x",activate_pixel.x },
            { "y",activate_pixel.y },
            { "z",activate_pixel.z },
            { "r",activate_pixel.r },
            { "g",activate_pixel.g },
            { "b",activate_pixel.b },
            { "a",activate_pixel.a },
            { "tag",activate_pixel.tag },
            { "blockSize",activate_pixel.block_size }
        });
    }
}

