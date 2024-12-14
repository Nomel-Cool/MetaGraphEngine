#include "PixelSpace.h"

void GraphAgency::LoadGraphs(const QString& model_name)
{
    std::lock_guard<std::mutex> lock(mtx4co);
    GraphFactory graph_factory;
    auto graph_model_coroutine_handler = graph_factory.OfferDynamicModel(model_name);
    // 存储协程句柄到舞台中
    // 由于ModelGenerator删除拷贝构造，而make_shared默认调用拷贝构造会报错，所以传参时传递std::move()，告知它使用移动构造。
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

const uint64_t& Hall::GetCurrentFPS() const
{
    return FPS;
}

const std::map<std::pair<std::size_t, std::size_t>, std::shared_ptr<OnePixel>>& Hall::GetStage() const
{
    return stage;
}

bool Hall::Disable(const std::pair<std::size_t, std::size_t>& coordinate)
{
    if (stage.find(coordinate) == stage.end())
        return false;
    stage[coordinate]->activate_flag = false;
    return true;
}

bool Hall::TransferPixelFrom(const std::pair<std::size_t, std::size_t>& coordinate_begin, OnePixel& dest_pixel)
{
    if (stage.find(coordinate_begin) == stage.end())
        return false;
    dest_pixel = *stage[coordinate_begin]; // 显式使用拷贝赋值符，用于移交所有权相关字段
    if (stage.find({dest_pixel.x, dest_pixel.y}) == stage.end())
        stage.insert(std::make_pair(std::make_pair(dest_pixel.x, dest_pixel.y), std::make_shared<OnePixel>(dest_pixel)));
    else
        stage[{dest_pixel.x, dest_pixel.y}] = std::make_shared<OnePixel>(dest_pixel);
    return true;
}

void Hall::PingStage(const std::size_t& x, const std::size_t& y, const std::size_t& graph_pos_in_list)
{
    auto key = std::make_pair(x, y);
    if (stage.find(key) != stage.end())
    {
        auto& pixel_ptr = stage[key];
        pixel_ptr->render_flag = true;
        pixel_ptr->activate_flag = true;
        pixel_ptr->x = x;
        pixel_ptr->y = y;
        pixel_ptr->cur_frame_id = frame_id;
        // 确保 graph_ids 中的元素唯一
        if (std::find(pixel_ptr->graph_ids.begin(), pixel_ptr->graph_ids.end(), graph_pos_in_list) == pixel_ptr->graph_ids.end())
            pixel_ptr->graph_ids.emplace_back(graph_pos_in_list);
    }
    else
    {
        // 基础设置
        OnePixel one_pixel;
        one_pixel.render_flag = true;
        one_pixel.activate_flag = true;
        one_pixel.x = x;
        one_pixel.y = y;
        one_pixel.cur_frame_id = frame_id;
        if (std::find(one_pixel.graph_ids.begin(), one_pixel.graph_ids.end(), graph_pos_in_list) == one_pixel.graph_ids.end())
            one_pixel.graph_ids.emplace_back(graph_pos_in_list);

        // 在堆上分配新内存存放 one_pixel，并由智能指针管理
        std::shared_ptr<OnePixel> sp_one_pixel = std::make_shared<OnePixel>(one_pixel);
        stage.insert(std::make_pair(key, sp_one_pixel));
    }
}

void Hall::NextFrame()
{
    ++frame_id;
}

// 计算一个数的所有因子
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

// 计算两个因子序列的交集
std::vector<std::size_t> GetCommonFactors(const std::vector<std::size_t>& factors1, const std::vector<std::size_t>& factors2)
{
    std::vector<std::size_t> common_factors;
    std::set_intersection(factors1.begin(), factors1.end(),
        factors2.begin(), factors2.end(),
        std::back_inserter(common_factors));
    return common_factors;
}

GraphStudio::GraphStudio(QObject* parent) : QObject(parent)
{
    sp_graph_agency = std::make_shared<GraphAgency>();
    sp_hall = std::make_shared<Hall>();
}

void GraphStudio::InitHall(const float& width, const float& height)
{
    // 逻辑像素空间比视图宽高都多一个像素，保证绘制的时候逻辑上能采到点，避免视图中有像素无法出现。
    sp_hall = std::make_shared<Hall>(static_cast<std::size_t>(width) + 1, static_cast<std::size_t>(height) + 1);
}

void GraphStudio::LayoutHall(const std::size_t& scale_extension)
{
    // 获取窗口逻辑宽高
    std::size_t logic_width = sp_hall->GetStageWidth();
    std::size_t logic_height = sp_hall->GetStageHeight();

    // 分别计算宽和高的所有因子
    std::vector<std::size_t> factors_width = GetFactors(logic_width);
    std::vector<std::size_t> factors_height = GetFactors(logic_height);

    // 获取公共因子
    std::vector<std::size_t> common_factors = GetCommonFactors(factors_width, factors_height);

    // 方格数量随下标增长而增长
    sp_hall->Layout(common_factors[scale_extension % common_factors.size()]);
}

void GraphStudio::RoleEmplacement(const QStringList& model_names)
{
    for (const QString& model_name : model_names)
    {
        pool.Enqueue(&GraphAgency::LoadGraphs, sp_graph_agency, model_name);
        // 或者调用 sp_graph_agency->LoadGraphs(model_name);
    }
}

QString GraphStudio::Display()
{
    // 定义 JSON 对象和映射
    json frames;
    std::unordered_map<uint64_t, json> frameMap;

    // 遍历 film 的数据并构建 frameMap
    for (const auto& pic : film.Fetch())
    {
        frameMap[pic.cur_frame_id].push_back({ {"x", pic.x}, {"y", pic.y}, {"blockSize", sp_hall->GetBlockSize()}});
    }

    // 将 frameMap 中的每一组点添加到 frames 中
    for (const auto& [id, points] : frameMap)
    {
        frames.push_back(points);
    }

    // 将 frames 转换为 JSON 字符串并返回
    return QString::fromStdString(frames.dump());
}

void GraphStudio::Launch()
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

            running = true;
        };

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, render_loop);
    timer->start(1000.0f / sp_hall->GetCurrentFPS()); // FPS ≈ 120
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
            sp_hall->PingStage(static_cast<std::size_t>(current_status["x"]), static_cast<std::size_t>(current_status["y"]), i);
        }
        catch (std::logic_error e)
        {
            std::cerr << "The model is done, break out!!!" << std::endl;
            return;
        }
    }
}

void GraphStudio::Interact()
{
    const auto& pixels = sp_hall->GetStage();

    for (const auto& pixel : pixels)
    {
        std::shared_ptr<OnePixel> pixel_object = pixel.second;
        if (pixel_object->render_flag != true)
            continue;
            /****************************** 对该像素的所有者，算出它们的下一个状态 *****************************************/
            json initial_status, current_status, current_input, terminate_status;
            OnePixel dest_pixel;
            float g = 9.8, v = 0, FPS = sp_hall->GetCurrentFPS(), delta_t = 1.0f / FPS, s = 0, cur_x = 0, cur_y = 0;
            for (const auto& indice : pixel.second->graph_ids)
            {
                const auto& automata_param = GetAutomataInfoAt(indice);
                std::tie(initial_status, current_status, current_input, terminate_status) = automata_param;
                if (current_status.empty() || current_input.empty())
                    continue;
                // 必须假设每个模型的自动机是能够跟当前法则交互的，即它具有那些属性（速度、加速度、质量等等词条）
                // 假设此处有引力场，且此处只对一个模型的位置属性进行交互
                // 位置的计算可以由法则给出，也可以把法则属性传递给模型计算后在下一次到来时给出，此处采用前者
                g = 9.8f;
                v = current_input["velocity"];
                s = std::abs(v * delta_t + 0.5f * g * delta_t * delta_t);
                cur_x = current_status["x"];
                cur_y = current_status["y"];
				s = v >= 0 ? s : -s;
                if (cur_y >= s) 
                {
                    // 还没有碰撞到地面，正常更新速度和位移
                    cur_y -= s;
                    v += g * delta_t;
                }
                else
                {
					if(v >= 0)
					{
						// 到达地面，计算剩余的位移和速度更新
						float time_to_ground = std::sqrt(2 * cur_y / g);
						// 速度到达地面时
						v += g * time_to_ground;
						// 碰撞后的反弹速度衰减
						v = -v * 0.8f;
					}
					else
						v += g * delta_t;
                    // 位置到地面
                    cur_x -= (cur_x == 0 ? 0 : 1);
                    cur_y = v < 0 ? cur_y + s : 0;
                }

                // 更新current_status
                current_status["x"] = cur_x;
                current_status["y"] = cur_y;
                current_input["velocity"] = v;
                dest_pixel.x = cur_x;
                dest_pixel.y = cur_y;
                SetAutomataInfoAt(indice, std::make_tuple(initial_status, current_status, current_input, terminate_status));
            }
        /****************************** 执行移交手续 *****************************************/
        bool displace_result = sp_hall->TransferPixelFrom(pixel.first, dest_pixel);
        if (!displace_result)
            std::cerr << "The displacement is out of range." << std::endl;
    }
}

void GraphStudio::UpdateGraphList()
{
    for (const auto& pixel : sp_hall->GetStage())
    {
        std::shared_ptr<OnePixel> specific_pixel = pixel.second;
        if (!specific_pixel->activate_flag)
            continue;
        sp_hall->Disable(pixel.first); // 置activate_flag为false
    }
    sp_graph_agency->UpdateGraphs(); // 返回协程算法
}

void GraphStudio::SnapShot()
{
    auto cur_id = sp_hall->GetCurrentFrameID();
    for (auto& pixel : sp_hall->GetStage())
    {
        std::shared_ptr<OnePixel> specific_pixel = pixel.second;
        if (specific_pixel->cur_frame_id == cur_id && specific_pixel->render_flag)
            film.Store(*specific_pixel);
    }
    sp_hall->NextFrame();
}

std::tuple<json, json, json, json> GraphStudio::GetAutomataInfoAt(std::size_t indice)
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

void GraphStudio::SetAutomataInfoAt(std::size_t indice, const std::tuple<json,json,json,json>& automata_status)
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

void CompressedFrame::Store(const OnePixel& one_pixel)
{
    frames.emplace_back(one_pixel);
}

const std::vector<OnePixel>& CompressedFrame::Fetch()
{
    return frames;
}

OnePixel& OnePixel::operator=(OnePixel& other)
{
    if (this != &other)
    {
        x = other.x;
        y = other.y;
        activate_flag = other.activate_flag;
        render_flag = other.render_flag;
        cur_frame_id = other.cur_frame_id;
        graph_ids = other.graph_ids; // 所有图元接受同样的法则，所以所有权完全移交没问题

        // 移交后更新源像素
        other.graph_ids.clear();
        other.render_flag = false;
        other.activate_flag = false;
    }
    return *this;
}