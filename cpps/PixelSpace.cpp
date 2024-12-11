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

std::vector<std::shared_ptr<ModelGenerator<SingleAutomata>>>& GraphAgency::GetGraphs()
{
    return graph_series_cache;
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

Hall::Hall(const std::size_t& stage_width, const std::size_t& stage_height)
{
    this->stage_width = stage_width;
    this->stage_height = stage_height;
}

void Hall::Layout(const std::size_t& block_size)
{
    this->block_size = block_size;
}

const std::size_t& Hall::GetStageHeight() const
{
    return stage_height;
}

const std::size_t& Hall::GetStageWidth() const
{
    return stage_width;
}

const std::size_t& Hall::GetBlockSize() const
{
    return block_size;
}

const uint64_t& Hall::GetCurrentFrameID() const
{
    return frame_id;
}

const std::map<std::pair<std::size_t, std::size_t>, OnePixel>& Hall::GetStage() const
{
    return stage;
}

void Hall::NextFrame()
{
    ++frame_id;
}

void Hall::PingStage(const std::size_t& x, const std::size_t& y, const std::size_t& graph_pos_in_list)
{
    if (stage.find({ x,y }) != stage.end())
    {
        stage[{x, y}].pre_frame_id = stage[{x, y}].cur_frame_id;
        stage[{x, y}].cur_frame_id = frame_id;
        stage[{x, y}].graph_ids.emplace_back(graph_pos_in_list);
    }
    else
    {
        OnePixel one_pixel;
        one_pixel.x = x;
        one_pixel.y = y;
        one_pixel.cur_frame_id = frame_id;
        one_pixel.pre_frame_id = frame_id - 1; // ����õ�û�б��޸ģ���Ӧ�ñ���Ӱ����������ʹ�ó�ʼ��֡idΪ0
        one_pixel.graph_ids.emplace_back(graph_pos_in_list);
        SetRule(x, y);
        stage.insert(std::make_pair(std::make_pair(x, y), one_pixel));
    }
}

bool Hall::SetRule(const std::size_t& x, const std::size_t& y, int pos)
{
    if (stage.find({ x,y }) == stage.end())
        return false;
    // Todo: Ŀǰ����δ�����϶����Ǵ�һ���̶�Դ����ȡ��ֱ���������汻[](){}����λ��
    if (pos == -1)
    {
        stage[{x, y}].rules.emplace_back([]() {});
        return true;
    }
    else
    {
        std::cerr << "Todo: WTF, I don't know how to emplace it." << std::endl;
        return true;
    }
}

void GraphStudio::UpdateGraphList()
{
    sp_graph_agency->UpdateGraphs();
}

void GraphStudio::SnapShot()
{
    auto cur_id = sp_hall->GetCurrentFrameID();
    for (auto& pixel : sp_hall->GetStage())
    {
        if (pixel.second.pre_frame_id == cur_id - 1)
            film.Store(pixel.second);
    }
    sp_hall->NextFrame();
}

void GraphStudio::InitHall(const float& width, const float& height)
{
    // �߼����ؿռ����ͼ��߶���һ�����أ���֤���Ƶ�ʱ���߼����ܲɵ��㣬������ͼ���������޷����֡�
    sp_hall = std::make_shared<Hall>(static_cast<std::size_t>(width) + 1, static_cast<std::size_t>(height) + 1);
}

// ����һ��������������
std::vector<std::size_t> GetFactors(std::size_t n)
{
    std::vector<std::size_t> factors;
    for (std::size_t i = 1; i * i <= n; ++i) {
        if (n % i == 0) {
            factors.push_back(i);
            if (i != n / i) {
                factors.push_back(n / i);
            }
        }
    }
    std::sort(factors.begin(), factors.end());
    return factors;
}

// ���������������еĽ���
std::vector<std::size_t> GetCommonFactors(const std::vector<std::size_t>& factors1, const std::vector<std::size_t>& factors2) 
{
    std::vector<std::size_t> common_factors;
    std::set_intersection(factors1.begin(), factors1.end(),
        factors2.begin(), factors2.end(),
        std::back_inserter(common_factors));
    return common_factors;
}

void GraphStudio::LayoutHall(const std::size_t& scale_extension)
{
    // ��ȡ�����߼����
    std::size_t logic_width = sp_hall->GetStageWidth();
    std::size_t logic_height = sp_hall->GetStageHeight();

    // �ֱ�����͸ߵ���������
    std::vector<std::size_t> factors_width = GetFactors(logic_width);
    std::vector<std::size_t> factors_height = GetFactors(logic_height);

    // ��ȡ��������
    std::vector<std::size_t> common_factors = GetCommonFactors(factors_width, factors_height);

    // �����������±�����������
    sp_hall->Layout(common_factors[scale_extension % common_factors.size()]);
}

void GraphStudio::RoleEmplacement(const QStringList& model_names)
{
    for (const QString& model_name : model_names)
    {
        pool.Enqueue(&GraphAgency::LoadGraphs, sp_graph_agency, model_name);
        // ���ߵ��� sp_graph_agency->LoadGraphs(model_name);
    }
}

void GraphStudio::Launch()
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

        StandBy(); // ���ƶ���
        Render(); // �������
        UpdateGraphList(); // ��������
        SnapShot(); // ��Ϊ����

        running = true;
    };

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, render_loop);
    timer->start(33); // FPS �� 30
}

QString GraphStudio::Display()
{
    // ���� JSON �����ӳ��
    json frames;
    std::unordered_map<uint64_t, json> frameMap;

    // ���� film �����ݲ����� frameMap
    for (const auto& pic : film.Fetch())
    {
        frameMap[pic.cur_frame_id].push_back({ {"x", pic.x}, {"y", pic.y}, {"blockSize", sp_hall->GetBlockSize()}});
    }

    // �� frameMap �е�ÿһ�����ӵ� frames ��
    for (const auto& [id, points] : frameMap)
    {
        frames.push_back(points);
    }

    // �� frames ת��Ϊ JSON �ַ���������
    return QString::fromStdString(frames.dump());
}



void GraphStudio::StandBy()
{
    auto& graph_list = sp_graph_agency->GetGraphs();
    for (std::size_t i = 0; i < graph_list.size(); ++i)
    {
        try
        {
            json current_status, current_input, terminate_status;
            auto str_cur_pos = graph_list[i]->GetValue().current_status;
            auto str_cur_input = graph_list[i]->GetValue().current_input;
            auto str_final_pos = graph_list[i]->GetValue().terminate_status;
            if (!file_manager.TransStr2JsonObject(str_cur_pos, current_status))
            {
                std::cerr << "failed to parse json: " << str_cur_pos << std::endl;
                return;
            }
            if (!file_manager.TransStr2JsonObject(str_cur_input, current_input))
            {
                std::cerr << "failed to parse json: " << str_cur_input << std::endl;
                return;
            }
            if (!file_manager.TransStr2JsonObject(str_final_pos, terminate_status))
            {
                std::cerr << "failed to parse json: " << str_final_pos << std::endl;
                return;
            }

            sp_hall->PingStage(static_cast<std::size_t>(current_status["x"]), static_cast<std::size_t>(current_status["y"]), i);
        }
        catch (std::logic_error e)
        {
            std::cerr << "The model is done, break out!!!" << std::endl;
            return;
        }
    }
}

void GraphStudio::Render()
{
    auto graph_list = sp_graph_agency->GetGraphs();
    auto pixels = sp_hall->GetStage();
    for (auto& pixel : pixels)
    {
        if (pixel.second.cur_frame_id == sp_hall->GetCurrentFrameID())
        {
            pixel.second.Execute(sp_graph_agency, sp_hall);
        }
    }
}

void CompressedFrame::Store(const OnePixel& one_pixel)
{
    frames.emplace_back(one_pixel);
}

const std::vector<OnePixel>& CompressedFrame::Fetch()
{
    return frames;
}

void OnePixel::Execute(std::shared_ptr<GraphAgency> p_agency, std::shared_ptr<Hall> p_hall)
{
    std::cout << "Hello From OnePixel::Execute" << std::endl;
    // ��ת���� ���ݵ�ǰ�����漰��ͼԪ���������е��ȡ�
    for (const auto& indice : graph_ids)
    {

    }
}
