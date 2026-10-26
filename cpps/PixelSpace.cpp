#include "PixelSpace.h"

#include <QMetaObject>

#include <chrono>
#include <thread>

GraphStudio::GraphStudio(QObject* parent) : QObject(parent)
{
    sp_frame_id_manager = std::make_shared<FrameIDManager>();
    sp_frame_generate_interval_manager = std::make_shared<FrameGenerateIntervalManager>();
    sp_filmname_manager = std::make_shared<FilmNameManager>();

    auto factory = std::make_shared<GraphFactory>();
    auto cohandler_cache = std::make_shared<ThreadSafeGraphCache>();
    auto model_loader = std::make_unique<GraphModelLoader>(factory);
    auto corotine_manager = std::make_unique<CoroutineExecutor>(cohandler_cache);
    sp_graph_agency = std::make_shared<GraphAgency>(std::move(model_loader), cohandler_cache, std::move(corotine_manager));

    sp_hall = std::make_shared<Hall>(sp_frame_id_manager);
    sp_law = std::make_shared<Law>();
    sp_timer = std::make_shared<QTimer>(this);
    sp_gl_screen = std::make_shared<GLScreen>();
    sp_film_static_storage = std::make_shared<FilmNameMap>();
    sp_film_realtime_storage = std::make_shared<TaskModelFrameQueue>();
    sp_realtime_photographer = std::make_shared<RealTimePhotoGrapher>(sp_film_realtime_storage);
    sp_static_photographer = std::make_shared<StaticPhotoGrapher>(sp_film_static_storage);

    std::unique_ptr<shabby::IXMLDocumentFactory> tinyxml_factory = std::make_unique<TinyXMLDocumentFactory>();
    sp_file_manager = std::make_shared<FileManager>(std::move(tinyxml_factory));
}

void GraphStudio::InitWindow(int width, int height, float cameraX, float cameraY, float cameraZ, bool perspective_type, bool view_lock)
{
    if (width <= 0 || height <= 0)
    {
        emit operationFailed(QStringLiteral("Output width and height must be positive."));
        return;
    }
    sp_gl_screen->InitScreen(width, height);
    sp_gl_screen->InitCamara(glm::vec3(cameraX, cameraY, cameraZ));
    sp_gl_screen->InitPerspectiveType(perspective_type);
    sp_gl_screen->SetViewLock(view_lock);
}

void GraphStudio::SetCameraControlMode(int mode)
{
    sp_gl_screen->SetCameraControlMode(mode);
}
void GraphStudio::SetFilmName(const QString& film_name)
{
    if (film_name.trimmed().isEmpty())
    {
        emit operationFailed(QStringLiteral("Film name cannot be empty."));
        return;
    }
    sp_filmname_manager->SetCurrentFilmName(film_name.toStdString());
}

void GraphStudio::RoleEmplacement(const QStringList& model_names)
{
    if (model_names.isEmpty())
    {
        emit operationFailed(QStringLiteral("Select at least one model before starting."));
        return;
    }
    sp_graph_agency->CleanGraphCache();
    const QStringList names = model_names;
    pool.Enqueue([this, names]()
    {
        try
        {
            for (const QString& name : names)
                sp_graph_agency->LoadGraphs(name);
        }
        catch (const std::exception& error)
        {
            const QString message = QString::fromUtf8(error.what());
            QMetaObject::invokeMethod(this, [this, message]() { emit modelsLoadFailed(message); }, Qt::QueuedConnection);
            return;
        }
        QMetaObject::invokeMethod(this, [this, names]() { emit modelsLoaded(names); }, Qt::QueuedConnection);
    });
}

void GraphStudio::Display(const QStringList& film_name_list)
{
    if (film_name_list.isEmpty())
    {
        emit operationFailed(QStringLiteral("No film selected."));
        return;
    }

    sp_gl_screen->ClearFrameData();
    bool has_frames = false;
    for (const auto& film_name : film_name_list)
    {
        auto compressed_pics = sp_film_static_storage->Fetch(film_name.toStdString());
        auto vec_pics = compressed_pics.GetFrames();
        std::cerr << "Film " << film_name.toStdString() << " frames=" << vec_pics.size() << std::endl;
        if (vec_pics.empty())
            continue;
        has_frames = true;
        std::vector<CubePixel> vec_cubes;
        for (const OnePixel& pic : vec_pics)
            vec_cubes.emplace_back(pic);
        sp_gl_screen->SetVerticesData(vec_cubes);
    }

    if (!has_frames)
    {
        emit operationFailed(QStringLiteral("The selected film has no frames. Generate it again."));
        emit playbackStopped();
        return;
    }

    emit playbackStarted(film_name_list.first());
    sp_gl_screen->Rendering();
    emit playbackStopped();
}

void GraphStudio::Launch()
{
    if (sp_graph_agency->Empty())
    {
        emit operationFailed(QStringLiteral("No models loaded. Select models and try again."));
        return;
    }

    disconnect(sp_timer.get(), &QTimer::timeout, nullptr, nullptr);
    auto render_loop = [this]()
    {
        static bool running = true;
        if (!running)
            return;
        running = false;

        StandBy();
        Interact();
        FixedStage();
        SnapShot();
        TidyUp();
        UpdateGraphList();

        running = true;
    };
    connect(sp_timer.get(), &QTimer::timeout, render_loop);
    sp_timer->start(sp_frame_generate_interval_manager->GetFrameGenerationInterval());
    emit engineStarted(QStringLiteral("offline"));
}

void GraphStudio::RealTimeRender()
{
    if (sp_graph_agency->Empty())
    {
        emit operationFailed(QStringLiteral("No models loaded. Select models and try again."));
        return;
    }

    emit engineStarted(QStringLiteral("realtime"));
    sp_gl_screen->is_running = true;
    realtime_running = true;
    try
    {
        std::thread tmp1 = std::thread([this]() {
            sp_gl_screen->RealTimeRendering(sp_film_realtime_storage);
            sp_gl_screen->is_running = false;
        });

        std::thread tmp2 = std::thread([this]() {
            using Clock = std::chrono::high_resolution_clock;
            using Duration = std::chrono::duration<double, std::milli>;
            sp_realtime_photographer->SetUsageStatus(true);
            while (sp_gl_screen->is_running)
            {
                if (sp_graph_agency->Inspect())
                {
                    sp_gl_screen->is_running = false;
                    break;
                }
                auto loop_start_time = Clock::now();
                if (!sp_gl_screen->is_running)
                    break;
                StandBy();
                if (!sp_gl_screen->is_running)
                    break;
                RealTimeInteract();
                FixedStage();
                TidyUp();
                RealTimeSnapShot();
                UpdateGraphList();
                auto loop_end_time = Clock::now();
                Duration loop_duration = loop_end_time - loop_start_time;
                auto sleep_duration = std::chrono::milliseconds(sp_frame_generate_interval_manager->GetFrameGenerationInterval()) - loop_duration;
                if (sleep_duration.count() > 0)
                    std::this_thread::sleep_for(sleep_duration);
            }
            sp_realtime_photographer->SetUsageStatus(false);
            sp_film_realtime_storage->ClearMemo();
        });

        tmp1.join();
        tmp2.join();
    }
    catch (const std::exception& error)
    {
        emit operationFailed(QString::fromUtf8(error.what()));
    }
    realtime_running = false;
    emit engineStopped(QStringLiteral("realtime"));
}

void GraphStudio::Ceize()
{
    const std::string film_name = sp_filmname_manager->GetCurrentFilmName();
    sp_timer->stop();
    disconnect(sp_timer.get(), &QTimer::timeout, nullptr, nullptr);
    if (!film_name.empty())
    {
        sp_static_photographer->FilmDone(film_name);
        emit filmSaved(QString::fromStdString(film_name));
        emit filmTerminated(QString::fromStdString(film_name));
    }
    else
    {
        emit operationFailed(QStringLiteral("No active film. Set a film name before starting."));
    }
    sp_filmname_manager->ResetCurrentFilmName();
    emit engineStopped(QStringLiteral("offline"));
}

void GraphStudio::RoleDismiss()
{
    sp_timer->stop();
    disconnect(sp_timer.get(), &QTimer::timeout, nullptr, nullptr);
    sp_graph_agency->CleanGraphCache();
    sp_filmname_manager->ResetCurrentFilmName();
}

void GraphStudio::Stop()
{
    if (realtime_running)
    {
        sp_gl_screen->is_running = false;
        return;
    }
    const std::string film_name = sp_filmname_manager->GetCurrentFilmName();
    sp_timer->stop();
    disconnect(sp_timer.get(), &QTimer::timeout, nullptr, nullptr);
    if (!film_name.empty())
    {
        sp_static_photographer->FilmDone(film_name);
        emit filmSaved(QString::fromStdString(film_name));
        emit filmTerminated(QString::fromStdString(film_name));
    }
    sp_filmname_manager->ResetCurrentFilmName();
    emit engineStopped(QStringLiteral("offline"));
}

void GraphStudio::StandBy()
{
    if (sp_graph_agency->Inspect())
    {
        if (realtime_running)
            sp_gl_screen->is_running = false;
        return;
    }
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
            if (!current_status.is_array())
                return;
            std::vector<OnePixel> point_list;
            UpdatePixelsThroughAutomata(point_list, current_status);
            sp_hall->PingStage(point_list, i);
        }
        catch (std::logic_error)
        {
            if (realtime_running)
            {
                sp_gl_screen->is_running = false;
                return;
            }
            Stop();
        }
    }
}

void GraphStudio::Interact()
{
    sp_law->AffectOn<Gravity>(this);
}

void GraphStudio::FixedStage()
{
    const auto& pixels = sp_hall->GetStage();
    for (auto iter_pixel = pixels.begin(); iter_pixel != pixels.end(); ++iter_pixel)
    {
        if (iter_pixel->second->render_flag != true)
            continue;
        bool displace_result = sp_hall->TransferPixelFrom(iter_pixel->first);
        if (!displace_result)
            std::cerr << "The displacement is out of expectation." << std::endl;
    }
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
            json initial_status, current_status, current_input, terminate_status;
            std::tie(initial_status, current_status, current_input, terminate_status) = automata_param;
            current_status.clear();
            UpdateAutomataThroughPixels(current_status, pixels.second);
            automata_param = std::make_tuple(initial_status, current_status, current_input, terminate_status);
            SetAutomataInfoAt(pixels.first, automata_param);
        }
        sp_graph_agency->UpdateGraphs();
    }
    catch (std::logic_error)
    {
        std::cerr << "The model is done, break out!!!" << std::endl;
        if (realtime_running)
        {
            sp_gl_screen->is_running = false;
            return;
        }
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
            throw std::logic_error("Failed to parse init_status JSON");
        if (!sp_file_manager->TransStr2JsonObject(graph_model.current_status, current_status))
            throw std::logic_error("Failed to parse current_status JSON");
        if (!sp_file_manager->TransStr2JsonObject(graph_model.current_input, current_input))
            throw std::logic_error("Failed to parse current_input JSON");
        if (!sp_file_manager->TransStr2JsonObject(graph_model.terminate_status, terminate_status))
            throw std::logic_error("Failed to parse terminate_status JSON");
    }
    catch (const std::logic_error&)
    {
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
    catch (std::logic_error)
    {
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
            { "x", activate_pixel.x },
            { "y", activate_pixel.y },
            { "z", activate_pixel.z },
            { "r", activate_pixel.r },
            { "g", activate_pixel.g },
            { "b", activate_pixel.b },
            { "a", activate_pixel.a },
            { "tag", activate_pixel.tag },
            { "blockSize", activate_pixel.block_size }
        });
    }
}