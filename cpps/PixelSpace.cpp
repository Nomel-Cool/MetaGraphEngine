#include "PixelSpace.h"

const uint64_t& Hall::GetCurrentFrameID() const
{
    return frame_id;
}

const int Hall::GetCurrentFrameGenerationInterval() const
{
    return frame_generation_interval;
}

const std::map<ThreeDCoordinate, std::shared_ptr<OnePixel>>& Hall::GetStage() const
{
    return stage;
}

std::map<ThreeDCoordinate, std::shared_ptr<OnePixel>>::iterator Hall::DeleteElementAt(const ThreeDCoordinate& pos)
{
    auto it = stage.find(pos);
    if (it != stage.end())
        return stage.erase(it);
    return stage.end();
}

bool Hall::TransferPixelFrom(const ThreeDCoordinate& coordinate_begin)
{
    if (stage.find(coordinate_begin) == stage.end())
        return false;

    std::shared_ptr<OnePixel> target_pixel = stage[coordinate_begin];

    target_pixel->cur_frame_id = GetCurrentFrameID();

    //�߼�˵����
    //        ��� seperate ����������û�з����ƶ�����ԭ�÷��룬��Ϊ��ֻ�漰����ɫ���޸ģ�����Ȼ�д���Ȩ�� target_pixel ��������
    //        ��� seperate �����������ƶ��������С�λ�ã���ֱ��ռ��
    //        ���� seperate �ƶ������Ѿ�����ռ�е�λ�ã���Ӧ�ö������κ�Ӱ�죨��ɫ��Ȼ�ɸ�λ��ԭ��ռ���߸��£����Ⱥ���Ӱ�죬��ԭ��ռ�����Կ��Եõ������������������仯��һ֡�ı仯�������޷������
    if (target_pixel->owners_info.size() > 1)
    {
        for (auto iter_owners_kv = target_pixel->owners_info.begin(); iter_owners_kv != target_pixel->owners_info.end();)
        {
            if (iter_owners_kv->second == nullptr)
            {
                ++iter_owners_kv;
                continue;
            }
            auto [it, inserted] = stage.try_emplace({ iter_owners_kv->second->x, iter_owners_kv->second->y, iter_owners_kv->second->z }, iter_owners_kv->second);
            if (!inserted)
            {
                if (iter_owners_kv->second->x != target_pixel->x || iter_owners_kv->second->y != target_pixel->y || iter_owners_kv->second->z != target_pixel->z)
                {
                    it->second->Merge(iter_owners_kv->second);
                    target_pixel->owners_info.erase(iter_owners_kv++); // ���������ȫ�գ���ᱻTidyUpʶ������
                }
                else
                    ++iter_owners_kv;
            }
            else
            {
                it->second->owners_info[it->second->tag] = iter_owners_kv->second;
                target_pixel->owners_info.erase(iter_owners_kv++);
            }
        }
        target_pixel->UpdateSurfaceByMainTag();
    }

    //�߼�˵����
    //        ���������ƫ����Ŀ�ĵ� available ��ֱ��ռ��
    //        ���������ƫ����Ŀ�ĵ� unavailable ���ںϵ��Ѵ��ڵ�������
    //        ���ն�Ҫ�����ƶ�ǰ�������ڴ�
    if (target_pixel->owners_info.size() == 1)
    {
        target_pixel->TryUpdatingInnerIfSinglePixel();
        auto& sp_dest_pixel = stage[{target_pixel->x, target_pixel->y, target_pixel->z}];
        if (sp_dest_pixel != target_pixel)
        {
            if (sp_dest_pixel != nullptr)
                sp_dest_pixel->Merge(target_pixel);
            else
                stage[{target_pixel->x, target_pixel->y, target_pixel->z}] = std::make_shared<OnePixel>(*target_pixel);
            stage[coordinate_begin] = nullptr; // �����ᱻ TidyUp ����ʶ������
        }
    }
    return true;
}

void Hall::PingStage(const std::vector<OnePixel>& ping_pixel_list, const std::size_t& graph_pos_in_list)
{
    for (const auto& ping_pixel : ping_pixel_list)
    {
        auto key = std::tie(ping_pixel.x, ping_pixel.y, ping_pixel.z);
        auto& sp_specific_pixel = stage[key];
        if (sp_specific_pixel)
        {
            OnePixel one_pixel;
            one_pixel.render_flag = true;
            one_pixel.x = ping_pixel.x;
            one_pixel.y = ping_pixel.y;
            one_pixel.z = ping_pixel.z;
            one_pixel.r = ping_pixel.r;
            one_pixel.g = ping_pixel.g;
            one_pixel.b = ping_pixel.b;
            one_pixel.a = ping_pixel.a;
            one_pixel.tag = ping_pixel.tag;
            one_pixel.block_size = ping_pixel.block_size;
            one_pixel.cur_frame_id = frame_id;
            auto sync_sp_pixel = std::make_shared<OnePixel>(one_pixel);
            auto wrap_sync_sp_pixel = sync_sp_pixel->owners_info[sync_sp_pixel->tag] = std::make_shared<OnePixel>(*sync_sp_pixel);
            sp_specific_pixel->Merge(sync_sp_pixel);
            checkin_sequence[graph_pos_in_list].push(wrap_sync_sp_pixel);
        }
        else
        {
            // ��������
            OnePixel one_pixel;
            one_pixel.render_flag = true;
            one_pixel.x = ping_pixel.x;
            one_pixel.y = ping_pixel.y;
            one_pixel.z = ping_pixel.z;
            one_pixel.r = ping_pixel.r;
            one_pixel.g = ping_pixel.g;
            one_pixel.b = ping_pixel.b;
            one_pixel.a = ping_pixel.a;
            one_pixel.tag = ping_pixel.tag;
            one_pixel.block_size = ping_pixel.block_size;
            one_pixel.cur_frame_id = frame_id;
            auto sync_sp_pixel = std::make_shared<OnePixel>(one_pixel);
            auto wrap_sync_sp_pixel = sync_sp_pixel->owners_info[sync_sp_pixel->tag] = std::make_shared<OnePixel>(*sync_sp_pixel);
            sp_specific_pixel = sync_sp_pixel;
            checkin_sequence[graph_pos_in_list].push(wrap_sync_sp_pixel);
        }
    }
}

std::map<std::size_t, std::vector<OnePixel>> Hall::CollectStage()
{
    std::map<std::size_t, std::vector<OnePixel>> collected_pixels;
    for (auto& sequencial_pixels : checkin_sequence)
    {
        while (!sequencial_pixels.second.empty())
        {
            collected_pixels[sequencial_pixels.first].emplace_back(*(sequencial_pixels.second.front()));
            sequencial_pixels.second.pop();
        }
    }
    checkin_sequence.clear();
    return collected_pixels;
}

void Hall::NextFrame()
{
    ++frame_id;
}

GraphStudio::GraphStudio(QObject* parent) : QObject(parent)
{
    auto factory = std::make_shared<GraphFactory>();
    auto cohandler_cache = std::make_shared<ThreadSafeGraphCache>();
    auto model_loader = std::make_unique<GraphModelLoader>(factory);
    auto corotine_manager = std::make_unique<CoroutineExecutor>(cohandler_cache);
    sp_graph_agency = std::make_shared<GraphAgency>(std::move(model_loader),cohandler_cache,std::move(corotine_manager));

    sp_hall = std::make_shared<Hall>();
    sp_law = std::make_shared<Law>();
    sp_timer = std::make_shared<QTimer>(this);
    sp_gl_screen = std::make_shared<GLScreen>();
    sp_filmname_manager = std::make_shared<FilmNameManager>();
    sp_film_static_storage = std::make_shared<FilmNameMap>();
    sp_film_realtime_storage = std::make_shared<TaskModelFrameQueue>();
    sp_realtime_photographer = std::make_shared<RealTimePhotoGrapher>(sp_film_realtime_storage);
    sp_static_photographer = std::make_shared<StaticPhotoGrapher>(sp_film_static_storage);

    // ���ʹ������������XML�������޸Ĺ���ָ�뼴��
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
        // ���ߵ��� sp_graph_agency->LoadGraphs(model_name);
    }
}

void GraphStudio::Display(const QStringList& film_name_list)
{
    if (film_name_list.empty())
        return;
    // ����������Ҫ���ŵ�֡������
    for (const auto& film_name : film_name_list)
    {
        std::vector<CubePixel> vec_cubes;
        auto compressed_pics = sp_film_static_storage->Fetch(film_name.toStdString());
        auto vec_pics = compressed_pics.GetFrames();
        for (const OnePixel& pic : vec_pics)
            vec_cubes.emplace_back(pic);
        sp_gl_screen->SetVerticesData(vec_cubes); // ��֡˳�����ö�������
    }

    // �������� �� ִ����Ⱦ
    sp_gl_screen->Rendering();

}

void GraphStudio::Launch()
{
    try
    {
        // ÿ1/30��Ϊһ��ʱ��Ƭ���ڴ�ʱ��Ƭ����Ҫ����
        // 1.�������Ҫ����for (auto& model : sp_graph_agency->GetGraphs())
        // 2.ȡmodel��CurrentStatus�еĵ㣬ִ��stage[{x,y}]��rule�����ܻ��޸�model��������Ϣ
        // 3.����ÿ���̸߳���һ��ͼԪ�����Բ����ٿ��̻߳�ͼ��ÿ��ͼԪ���Ѿ��ִ�������
        // 4.����stageѹ����Ⱦ����Ϣ
        // 5.modelִ��resume
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

            /**************** �ݶ������ļ��� ***************/
            StandBy(); // ���ƶ���
            Interact(); // �������
            UpdateGraphList(); // ��������
            SnapShot(); // ��Ϊ����
            TidyUp(); // �������Ⱦ����

            running = true;
        };
        connect(sp_timer.get(), &QTimer::timeout, render_loop);
        sp_timer->start(sp_hall->GetCurrentFrameGenerationInterval()); // FPS �� 120
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
        sp_gl_screen->is_running = false; // tmp1 �˳�ʱ��֪ͨ tmp2 �˳�
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
                if (!sp_gl_screen->is_running) break; // ��� tmp1 �˳������˳�ѭ��

                ///**************** �ݶ������ļ��� ***************/
                StandBy();          // ���ƶ���
                RealTimeInteract(); // ���ݲ���ʵʱ�������
                UpdateGraphList();  // ��������
                TidyUp();           // �������Ⱦ����
                RealTimeSnapShot(); // ��Ϊʵʱ����
                auto loop_end_time = Clock::now();
                Duration loop_duration = loop_end_time - loop_start_time;
                auto sleep_duration = std::chrono::milliseconds(sp_hall->GetCurrentFrameGenerationInterval()) - loop_duration; // ����Ϊ 58 FPS
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
            // ����Э�飬�����ֶξ�Ϊ����
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
            // �˴������ֶ����µ�ǰ��Ⱦ����Զ�����Ϣ
            // �Ҳ�û�����߽��⣬�����Ǹ��յ�json����һ����ȡ��json�ͻ��׳��쳣ʹ�ó��������������ȷ�����ͼԪ��ͼ��ȷʵ���и��ֶ�
            json initial_status, current_status, current_input, terminate_status;
            std::tie(initial_status, current_status, current_input, terminate_status) = automata_param;
            // ��Ϊ���������Զ�������Щ�ֶΣ�Ӧ��������ָ�봫���������������ֶ�����λ��
            current_status.clear();
            UpdateAutomataThroughPixels(current_status, pixels.second);
            /*******************************************************************/
            automata_param = std::make_tuple(initial_status, current_status, current_input, terminate_status);
            SetAutomataInfoAt(pixels.first, automata_param);
        }
        sp_graph_agency->UpdateGraphs(); // ����Э���㷨
    }
    catch (std::logic_error)
    {
        std::cerr << "The model is done, break out!!!" << std::endl;
        Stop();
    }
}

void GraphStudio::SnapShot()
{
    auto cur_id = sp_hall->GetCurrentFrameID();
    for (auto& pixel : sp_hall->GetStage())
    {
        std::shared_ptr<OnePixel> specific_pixel = pixel.second;
        if (specific_pixel->cur_frame_id == cur_id && specific_pixel->render_flag)
            sp_static_photographer->Filming(*specific_pixel);
    }
    sp_hall->NextFrame();
}

void GraphStudio::RealTimeSnapShot()
{
    auto cur_id = sp_hall->GetCurrentFrameID();
    for (auto& pixel : sp_hall->GetStage())
    {
        std::shared_ptr<OnePixel> specific_pixel = pixel.second;
        if (specific_pixel->cur_frame_id == cur_id && specific_pixel->render_flag)
            sp_realtime_photographer->Filming(*specific_pixel);
    }
    sp_realtime_photographer->FilmDone();
    sp_hall->NextFrame();
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

