#include "PixelSpace.h"

void GraphAgency::LoadGraphs(const QString& model_name)
{
    std::lock_guard<std::mutex> lock(mtx4co);
    GraphFactory graph_factory;
    auto graph_model_coroutine_handler = graph_factory.OfferDynamicModel(model_name);
    // �洢Э�̾������̨��
    // ����ModelGeneratorɾ���������죬��make_sharedĬ�ϵ��ÿ�������ᱨ�����Դ���ʱ����std::move()����֪��ʹ���ƶ����졣
    auto sp_co_graph_handler = std::make_shared<ModelGenerator<SingleAutomata>>(std::move(graph_model_coroutine_handler));
    graph_series_cache.emplace_back(sp_co_graph_handler);
}

void GraphAgency::UpdateGraphs()
{
    for (auto& graph : graph_series_cache)
        graph->Resume();
}

const std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GraphAgency::GetGraphs()
{
    return graph_series_cache;
}

bool GraphAgency::Inspect()
{
    bool check_result = true;
    for (const auto& actor : graph_series_cache)
        check_result &= actor->Done();
    return check_result;
}

void GraphAgency::CleanGraphCache()
{
    if(!graph_series_cache.empty())
        graph_series_cache.clear();
}

bool GraphAgency::Empty()
{
    return graph_series_cache.empty();
}

const uint64_t& Hall::GetCurrentFrameID() const
{
    return frame_id;
}

const uint64_t& Hall::GetCurrentFPS() const
{
    return FPS;
}

const std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>>& Hall::GetStage() const
{
    return stage;
}

std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>>::iterator Hall::DeleteElementAt(const std::pair<std::size_t, std::size_t>& pos)
{
    auto it = stage.find(pos);
    if (it != stage.end())
        return stage.erase(it);
    return stage.end();
}

bool Hall::Disable(const std::pair<std::size_t, std::size_t>& coordinate, std::size_t graph_id)
{
    if (stage.find(coordinate) == stage.end())
        return false;
    stage[coordinate]->graph_ids[graph_id] = false;
    return true;
}

bool Hall::TransferPixelFrom(const std::pair<std::size_t, std::size_t>& coordinate_begin)
{
    if (stage.find(coordinate_begin) == stage.end())
        return false;
    OnePixel dest_pixel;
    dest_pixel = *stage[coordinate_begin]; // ��ʽʹ�ÿ�����ֵ���������ƽ�����Ȩ����ֶ�
    dest_pixel.cur_frame_id = GetCurrentFrameID(); // �ƽ�������render_flag����true����Ҫ����Ϊ��ǰ֡�����ܱ�����
    if (stage.find({dest_pixel.x, dest_pixel.y}) == stage.end())
        stage.insert(std::make_pair(std::make_pair(dest_pixel.x, dest_pixel.y), std::make_shared<OnePixel>(dest_pixel)));
    else
        stage[{dest_pixel.x, dest_pixel.y}] = std::make_shared<OnePixel>(dest_pixel);
    return true;
}

void Hall::PingStage(const std::vector<OnePixel>& ping_pixel_list, const std::size_t& graph_pos_in_list)
{
    for (const auto& ping_pixel : ping_pixel_list)
    {
        auto key = std::make_pair(ping_pixel.x, ping_pixel.y);
        if (stage.find(key) != stage.end())
        {
            auto& pixel_ptr = stage[key];
            pixel_ptr->render_flag = true;
            pixel_ptr->x = ping_pixel.x;
            pixel_ptr->y = ping_pixel.y;
            pixel_ptr->r = ping_pixel.r;
            pixel_ptr->g = ping_pixel.g;
            pixel_ptr->b = ping_pixel.b;
            pixel_ptr->a = ping_pixel.a;
            pixel_ptr->block_size = ping_pixel.block_size;
            pixel_ptr->cur_frame_id = frame_id;
            // ȷ�� graph_ids �е�Ԫ��Ψһ
            pixel_ptr->graph_ids[graph_pos_in_list] = true;
            checkin_sequence[graph_pos_in_list].push(pixel_ptr);
        }
        else
        {
            // ��������
            OnePixel one_pixel;
            one_pixel.render_flag = true;
            one_pixel.x = ping_pixel.x;
            one_pixel.y = ping_pixel.y;
            one_pixel.r = ping_pixel.r;
            one_pixel.g = ping_pixel.g;
            one_pixel.b = ping_pixel.b;
            one_pixel.a = ping_pixel.a;
            one_pixel.block_size = ping_pixel.block_size;
            one_pixel.cur_frame_id = frame_id;
            one_pixel.graph_ids[graph_pos_in_list] = true;

            // �ڶ��Ϸ������ڴ��� one_pixel����������ָ�����
            std::shared_ptr<OnePixel> sp_one_pixel = std::make_shared<OnePixel>(one_pixel);
            stage.insert(std::make_pair(key, sp_one_pixel));
            checkin_sequence[graph_pos_in_list].push(sp_one_pixel);
        }
    }
}

std::map<std::size_t, std::vector<OnePixel>> Hall::CollectStage()
{
    std::map<std::size_t, std::vector<OnePixel>> collected_pixels;
    for (auto& pixel : checkin_sequence)
    {
        collected_pixels[pixel.first].emplace_back(*(pixel.second.front()));
        pixel.second.pop();
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
    sp_graph_agency = std::make_shared<GraphAgency>();
    sp_hall = std::make_shared<Hall>();
    sp_law = std::make_shared<Law>();
    sp_timer = std::make_shared<QTimer>(this);
    sp_gl_screen = std::make_shared<GLScreen>();
}

void GraphStudio::InitWindow(int width, int height)
{

}

void GraphStudio::SetFilmName(const QString& film_name)
{
    photo_grapher.RecordFilmName(film_name.toStdString());
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
    // ��������
    // (Todo)

    // ����������Ҫ���ŵ�֡������
    for (const auto& film_name : film_name_list)
    {
        auto compressed_pics = photo_grapher.Fetch(film_name.toStdString());
        auto vec_pics = compressed_pics.GetFrames();
        const auto& pin_pos = compressed_pics.GetPinPos();

        for (OnePixel pic : vec_pics)
        {
            pic.x += pin_pos.first;
            pic.y += pin_pos.second;
        }
    }

    // ����
    // (Todo)
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
        sp_timer->start(1000.0f / sp_hall->GetCurrentFPS()); // FPS �� 120
    }
    catch (const std::logic_error)
    {
        Stop();
    }
}

void GraphStudio::Ceize()
{
    sp_timer->stop();
    disconnect(sp_timer.get(), &QTimer::timeout, nullptr, nullptr);
    photo_grapher.Store();
}

void GraphStudio::RoleDismiss()
{
    sp_graph_agency->CleanGraphCache();
}

void GraphStudio::Stop()
{
    if (!sp_graph_agency->Inspect())
        return;
    sp_timer->stop();
    disconnect(sp_timer.get(), &QTimer::timeout, nullptr, nullptr);
    emit filmTerminated(QString::fromStdString(photo_grapher.GetCurrentFilmName()));
    photo_grapher.Store();
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
            if (!file_manager.TransStr2JsonObject(str_cur_pos, current_status))
            {
                std::cerr << "failed to parse json: " << str_cur_pos << std::endl;
                return;
            }
            // ����Э�飬�����ֶξ�Ϊ����
            if (!current_status.is_array())
                return;
            std::vector<OnePixel> point_list;
            for (const auto& point : current_status)
            {
                OnePixel ping_pixel;
                ping_pixel.x = point["x"];
                ping_pixel.y = point["y"];
                ping_pixel.r = point["r"];
                ping_pixel.g = point["g"];
                ping_pixel.b = point["b"];
                ping_pixel.a = point["a"];
                ping_pixel.block_size = point["blockSize"];
                point_list.emplace_back(ping_pixel);
            }
            sp_hall->PingStage(point_list, i);
        }
        catch (std::logic_error)
        {
            std::cerr << "The model is done, break out!!!" << std::endl;
            Stop();
        }
    }
}

void GraphStudio::Interact()
{
    sp_law->AffectOn<Gravity>(this);
}

void GraphStudio::UpdateGraphList()
{
    try
    {
        const auto activating_pixels = sp_hall->CollectStage();
        for (const auto& pixels : activating_pixels)
        {
            AutomataElements automata_param = GetAutomataInfoAt(pixels.first);
            // �˴������ֶ����µ�ǰ��Ⱦ����Զ�����Ϣ
            // �Ҳ�û�����߽��⣬�����Ǹ��յ�json����һ����ȡ��json�ͻ��׳��쳣ʹ�ó��������������ȷ�����ͼԪ��ͼ��ȷʵ���и��ֶ�
            json initial_status, current_status, current_input, terminate_status;
            std::tie(initial_status, current_status, current_input, terminate_status) = automata_param;
            // ��Ϊ���������Զ�������Щ�ֶΣ�Ӧ��������ָ�봫���������������ֶ�����λ��
            current_status.clear();
            for (const auto& activate_pixel : pixels.second)
            {
                current_status.push_back({
                    { "x",activate_pixel.x },
                    { "y",activate_pixel.y },
                    { "r",activate_pixel.r },
                    { "g",activate_pixel.g },
                    { "b",activate_pixel.b },
                    { "a",activate_pixel.a },
                    { "size",activate_pixel.block_size }
                });
                sp_hall->Disable({ activate_pixel.x, activate_pixel.y }, pixels.first); // ��activate_flagΪfalse
            }
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
            photo_grapher.Filming(*specific_pixel);
    }
    sp_hall->NextFrame();
}

void GraphStudio::TidyUp()
{
    for (auto iter = sp_hall->GetStage().begin(); iter != sp_hall->GetStage().end();)
        if (!iter->second->render_flag)
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

        if (!file_manager.TransStr2JsonObject(graph_model.init_status, init_status))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
            throw std::logic_error("Failed to parse init_status JSON");
        }
        if (!file_manager.TransStr2JsonObject(graph_model.current_status, current_status))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
            throw std::logic_error("Failed to parse current_status JSON");
        }
        if (!file_manager.TransStr2JsonObject(graph_model.current_input, current_input))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.current_input << std::endl;
            throw std::logic_error("Failed to parse current_input JSON");
        }
        if (!file_manager.TransStr2JsonObject(graph_model.terminate_status, terminate_status))
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

