#include "PixelSpace.h"

void GraphAgency::LoadGraphs(const QString& model_name)
{
    GraphFactory graph_factory;
    auto graph_model_coroutine_handler = graph_factory.OfferDynamicModel(model_name);
    // 存储协程句柄到舞台中
    // 由于ModelGenerator删除拷贝构造，而make_shared默认调用拷贝构造会报错，所以传参时传递std::move()，告知它使用移动构造。
    auto sp_co_graph_handler = std::make_shared<ModelGenerator<SingleAutomata>>(std::move(graph_model_coroutine_handler));
    graph_series_cache.emplace_back(sp_co_graph_handler);
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

void Hall::Layout(const std::size_t& block_size)
{
    // 只修改[0,0]位置的大小信息，其余像素使用懒更新
    stage[0][0].block_size = block_size;
}

std::size_t Hall::GetStageHeight()
{
    return stage.begin()->size();
}

std::size_t Hall::GetStageWidth()
{
    return stage.size();
}

OnePixel Hall::SetStage(std::size_t i, std::size_t j)
{
    stage[i][j].UpdatePos(i, j);
    stage[i][j].UpdateSize(stage[0][0].block_size);
    auto one_pixel = stage[i][j].ApplyRule(*this);
    return one_pixel;
}

std::vector<std::vector<OnePixel>>& Hall::GetStage()
{
    return stage;
}

void Hall::WhenAccessPixel(const std::size_t& i, const std::size_t& j)
{
    stage[i][j].UpdateSize(stage[0][0].block_size);
    // May be more default action when once access a pixel in [i,j]
}

void OnePixel::UpdatePos(const std::size_t& x, const std::size_t& y)
{
    this->x = x;
    this->y = y;
}

void OnePixel::UpdateSize(const std::size_t& block_size)
{
    this->block_size = block_size;
}

OnePixel OnePixel::ApplyRule(Hall& hall)
{
    auto stage_in_pixel = hall.GetStage();
    stage_in_pixel[x][y + 1].UpdatePos(x, y + 1);
    stage_in_pixel[x][y + 1].render_flag = true;
    return stage_in_pixel[x][y + 1];
}

void GraphStudio::InitHall(const float& width, const float& height)
{
    // 逻辑像素空间比视图宽高都多一个像素，保证绘制的时候逻辑上能采到点，避免视图中有像素无法出现。
    sp_hall = std::make_shared<Hall>(static_cast<std::size_t>(width) + 1, static_cast<std::size_t>(height) + 1);
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

bool GraphStudio::RoleEmplacement(const QString& model_name)
{
    // todo：入参会更改为模型名的数组，需要大量的调用loadgraph
    sp_graph_agency->LoadGraphs(model_name);
    return false;
}

void GraphStudio::Launch()
{
    if (sp_graph_agency->Empty())
    {
        std::cerr << "no actor..." << std::endl;
        return;
    }
    auto drawLoop = [this]() {
        static bool running = true;
        if (!running)
        {
            return;
        }
        running = false;
        for (auto& model : sp_graph_agency->GetGraphs())
        {
            try
            {
                json current_status, terminate_status;
                auto str_cur_pos = model->GetValue().current_status;
                auto str_final_pos = model->GetValue().terminate_status;
                if (!file_manager.TransStr2JsonObject(str_cur_pos, current_status))
                {
                    std::cerr << "failed to parse json: " << str_cur_pos << std::endl;
                    return;
                }
                if (!file_manager.TransStr2JsonObject(str_final_pos, terminate_status))
                {
                    std::cerr << "failed to parse json: " << str_final_pos << std::endl;
                    return;
                }
                float x = current_status["x"];
                float y = current_status["y"];
                auto one_pixel = sp_hall->SetStage(x, y);

                current_status["x"] = one_pixel.x;
                current_status["y"] = one_pixel.y;
                model->GetValue().current_status = current_status.dump();
                float tx = terminate_status["x"];
                float ty = terminate_status["y"];
                //terminate_status["x"] = tx + 1;
                terminate_status["y"] = ty + 1;
                model->GetValue().terminate_status = terminate_status.dump();

                emit drawPixeled(one_pixel.x, one_pixel.y, one_pixel.block_size);

                model->Resume();
            }
            catch (std::logic_error e)
            {
                std::cerr << "The model is done, break out!!!" << std::endl;
                return;
            }
        }
        running = true;
        };

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, drawLoop);
    timer->start(50); // 每 50 毫秒触发一次绘制
}

