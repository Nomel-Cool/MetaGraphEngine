#include "GraphGenerateAlgorithmLib.h"

BresenhamLineRenderer::BresenhamLineRenderer(std::shared_ptr<FileManager> file_manager)
{
	sp_file_manager = file_manager;
}
std::string BresenhamLineRenderer::Execute(const SingleAutomata& graph_model)
{
    json init_status, terminate_status;
    if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        return "{}";
    }
    if (!sp_file_manager->TransStr2JsonObject(graph_model.terminate_status, terminate_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.terminate_status << std::endl;
        return "{}";
    }
    std::vector<std::pair<float, float>> points;
    float x0 = init_status[0]["x"];
    float y0 = init_status[0]["y"];
    float xn = terminate_status[0]["x"];
    float yn = terminate_status[0]["y"];
    float dx = fabs(xn - x0);
    float dy = fabs(yn - y0);
    float stepX = (x0 < xn) ? 1 : -1;
    float stepY = (y0 < yn) ? 1 : -1;
    float x = x0;
    float y = y0;

    if (dx > dy) // |m| <= 1
    {
        float p = 2 * dy - dx;
        while (stepX > 0 ? x <= xn : x >= xn)
        {
            points.emplace_back(std::make_pair(x, y));
            if (p >= 0)
            {
                y += stepY;
                p -= 2 * dx;
            }
            x += stepX;
            p += 2 * dy;
        }
    }
    else // |m| > 1
    {
        float p = 2 * dx - dy;
        while (stepY > 0 ? y <= yn : y >= yn)
        {
            points.emplace_back(std::make_pair(x, y));
            if (p >= 0)
            {
                x += stepX;
                p -= 2 * dy;
            }
            y += stepY;
            p += 2 * dx;
        }
    }

    // Add the final point
    points.emplace_back(std::make_pair(xn, yn));

    json result;
    for (const auto& point : points)
        result.push_back({ {"point", {point.first, point.second}} });

    return result.dump();
}
ModelGenerator<SingleAutomata> BresenhamLineRenderer::Execute(SingleAutomata& graph_model)
{
    json init_status, current_status, current_input, terminate_status;

    // Convert graph_model status strings to JSON objects
    if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        co_return;
    }
    if (!sp_file_manager->TransStr2JsonObject(graph_model.current_status, current_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
        co_return;
    }
    if (!sp_file_manager->TransStr2JsonObject(graph_model.current_input, current_input))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.current_input << std::endl;
        co_return;
    }
    if (!sp_file_manager->TransStr2JsonObject(graph_model.terminate_status, terminate_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.terminate_status << std::endl;
        co_return;
    }

    // Initialize current_status as a JSON array if it's empty
    if (current_status.empty())
    {
        float init_x = init_status[0]["x"];
        float init_y = init_status[0]["y"];
        float init_z = init_status[0]["z"];
        float init_r = init_status[0]["r"];
        float init_g = init_status[0]["g"];
        float init_b = init_status[0]["b"];
        float init_a = init_status[0]["a"];
        std::size_t init_tag = current_input[0]["tagHead"];
        float init_size = init_status[0]["blockSize"];
        current_status[0]["x"] = init_x;
        current_status[0]["y"] = init_y;
        current_status[0]["z"] = init_z;
        current_status[0]["r"] = init_r;
        current_status[0]["g"] = init_g;
        current_status[0]["b"] = init_b;
        current_status[0]["a"] = init_a;
        current_status[0]["tag"] = init_tag + 1;
        current_status[0]["blockSize"] = init_size;
        graph_model.current_status = current_status.dump();
    }

    std::size_t tag = current_status[0]["tag"];
    float x0 = init_status[0]["x"];
    float y0 = init_status[0]["y"];
    float xn = terminate_status[0]["x"];
    float yn = terminate_status[0]["y"];
    float dx = fabs(xn - x0);
    float dy = fabs(yn - y0);
    float stepX = (x0 < xn) ? 1 : -1;
    float stepY = (y0 < yn) ? 1 : -1;
    float x = x0;
    float y = y0;

    if (dx > dy) // |m| <= 1
    {
        float p = 2 * dy - dx;
        while (stepX > 0 ? x <= xn : x >= xn)
        {
            co_yield graph_model;
            if (p >= 0)
            {
                y += stepY;
                p -= 2 * dx;
            }
            x += stepX;
            p += 2 * dy;

            current_status[0]["x"] = x;
            current_status[0]["y"] = y;
            current_status[0]["tag"] = tag++;
            graph_model.current_status = current_status.dump();
        }
    }
    else // |m| > 1
    {
        float p = 2 * dx - dy;
        while (stepY > 0 ? y <= yn : y >= yn)
        {
            co_yield graph_model;
            if (p >= 0)
            {
                x += stepX;
                p -= 2 * dy;
            }
            y += stepY;
            p += 2 * dx;

            current_status[0]["x"] = x;
            current_status[0]["y"] = y;
            current_status[0]["tag"] = tag++;
            graph_model.current_status = current_status.dump();
        }
    }
}

JustAPointRenderer::JustAPointRenderer(std::shared_ptr<FileManager> file_manager)
{
    sp_file_manager = file_manager;
}
std::string JustAPointRenderer::Execute(const SingleAutomata& graph_model)
{
    json init_status;
    if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        return "{}";
    }
    return init_status.dump();
}
ModelGenerator<SingleAutomata> JustAPointRenderer::Execute(SingleAutomata& graph_model)
{
    json init_status, current_status;
    if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        co_return;
    }
    if (!sp_file_manager->TransStr2JsonObject(graph_model.current_status, current_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
        co_return;
    }
    // 根据协议，所有自动机字段都是数组
    if (current_status.empty()) // 执行初始化
    {
        float init_x = init_status[0]["x"];
        float init_y = init_status[0]["y"];
        float init_z = init_status[0]["z"];
        float init_r = init_status[0]["r"];
        float init_g = init_status[0]["g"];
        float init_b = init_status[0]["b"];
        float init_a = init_status[0]["a"];
        std::size_t init_tag = init_status[0]["tag"];
        float init_size = init_status[0]["blockSize"];
        current_status[0]["x"] = init_x;
        current_status[0]["y"] = init_y;
        current_status[0]["z"] = init_z;
        current_status[0]["r"] = init_r;
        current_status[0]["g"] = init_g;
        current_status[0]["b"] = init_b;
        current_status[0]["a"] = init_a;
        current_status[0]["tag"] = init_tag;
        current_status[0]["blockSize"] = init_size;
        graph_model.current_status = current_status.dump();
    }
    while (true)
    {
        co_yield graph_model;
        if (!sp_file_manager->TransStr2JsonObject(graph_model.current_status, current_status))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
            co_return;
        }
        if (current_status[0]["x"] < 0)
            break;
    }
}

BresenhamEllipseRenderer::BresenhamEllipseRenderer(std::shared_ptr<FileManager> file_manager)
{
    sp_file_manager = file_manager;
}
std::string BresenhamEllipseRenderer::Execute(const SingleAutomata& graph_model)
{
    json init_status, terminate_status;
    if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        return "{}";
    }
    if (!sp_file_manager->TransStr2JsonObject(graph_model.terminate_status, terminate_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.terminate_status << std::endl;
        return "{}";
    }

    // 若不需要动画效果，去除points_part_1~points_part_8
    std::vector<std::pair<float, float>> points_part;
    std::vector<std::pair<float, float>> points_part_1;
    std::vector<std::pair<float, float>> points_part_2;
    std::vector<std::pair<float, float>> points_part_3;
    std::vector<std::pair<float, float>> points_part_4;

    // 若不需要动画效果，更改以下points_part_1~points_part_8为points_part即可。
    auto add_circle_points = [&](int xc, int yc, int x, int y) {
        points_part_1.push_back({ xc + x, yc + y });
        points_part_2.push_back({ xc - x, yc + y });
        points_part_3.push_back({ xc - x, yc - y });
        points_part_4.push_back({ xc + x, yc - y });
        };

    float xc = init_status["x"];
    float yc = init_status["y"];
    float a = terminate_status["a"];
    float b = terminate_status["b"];

    float x = 0;
    float y = b;
    float dd = 0;
    float index = 1; // 如果设为0，则初始点会被纳入，会有一个重复的像素
    while (x <= a && y >= 0) // 必须指定x和y的边界，否则可能因为曲线过于陡峭使得控制系数增长极度缓慢时，造成内外while的死循环
    {
        dd = 2 * b * index * a * a - index * index * a * a;
        while (b * b * x * x <= dd) // 取 <= 会偏向取平行于x轴，取 < 会偏向取平行y轴
        {
            add_circle_points(xc, yc, x, y);
            x++;
        }
        add_circle_points(xc, yc, x, y - 1); // 当y方向曲线过于陡峭的时候，一个x可能跨过多个下降标志，所以决策每次x跳过一个下降标志，需要直接下降。
        index++;
        y--;
    }

    // 若不需要动画效果，删除以下所有insert
    points_part.insert(points_part.end(), points_part_4.rbegin(), points_part_4.rend());
    points_part.insert(points_part.end(), points_part_3.begin(), points_part_3.end());
    points_part.insert(points_part.end(), points_part_2.rbegin(), points_part_2.rend());
    points_part.insert(points_part.end(), points_part_1.begin(), points_part_1.end());

    json result;
    for (const auto& point : points_part)
        result.push_back({ {"point", {point.first, point.second}} });

    return result.dump();
}
ModelGenerator<SingleAutomata> BresenhamEllipseRenderer::Execute(SingleAutomata& graph_model)
{
    return ModelGenerator<SingleAutomata>();
}

PartitionBezierCurveRenderer::PartitionBezierCurveRenderer(std::shared_ptr<FileManager> file_manager)
{
    sp_file_manager = file_manager;
}
std::string PartitionBezierCurveRenderer::Execute(const SingleAutomata& graph_model)
{
    json init_status;
    if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        return "{}";
    }

    int n = init_status.size() - 1; // 贝塞尔曲线的阶数为控制点数 - 1

    // 贝塞尔曲线方程
    auto bezier_formula = [&](float t) {
        float x = 0.0;
        float y = 0.0;
        for (int i = 0; i <= n; ++i) {
            float binom = binomial_coeff(n, i);
            float term = std::pow(1 - t, n - i) * std::pow(t, i);
            x += binom * term * init_status[i]["x"];
            y += binom * term * init_status[i]["y"];
        }
        return std::make_pair(x, y);
        };

    // 求最左和最右坐标
    float left = init_status[0]["x"];
    float right = init_status[0]["x"];
    for (const auto& p : init_status) {
        if (p["x"] < left) left = p["x"];
        if (p["x"] > right) right = p["x"];
    }

    std::vector<std::pair<float, float>> points_part;

    // 平均分成100份
    int num_partitions = 100;
    for (int i = 0; i <= num_partitions; ++i)
    {
        float t = static_cast<float>(i) / num_partitions;
        auto point = bezier_formula(t);
        float x = std::round(point.first);
        float y = std::round(point.second);
        points_part.emplace_back(x, y);
    }

    json result;
    for (const auto& point : points_part)
        result.push_back({ {"point", {point.first, point.second}} });

    return result.dump();
}
ModelGenerator<SingleAutomata> PartitionBezierCurveRenderer::Execute(SingleAutomata& graph_model)
{
    return ModelGenerator<SingleAutomata>();
}
int PartitionBezierCurveRenderer::binomial_coeff(int n, int k)
{
    if (k == 0 || k == n) return 1;
    if (k > n - k) k = n - k;
    int res = 1;
    for (int i = 0; i < k; ++i) {
        res *= (n - i);
        res /= (i + 1);
    }
    return res;
}

CellAutomataFrame_0::CellAutomataFrame_0(std::shared_ptr<FileManager> file_manager)
{
    sp_file_manager = file_manager;
    Initialize();
}
std::string CellAutomataFrame_0::Execute(const SingleAutomata& graph_model)
{
    return std::string();
}
ModelGenerator<SingleAutomata> CellAutomataFrame_0::Execute(SingleAutomata& graph_model)
{
    int n = 2;
    while (true) 
    {
        json current_status;
        // 把培养皿映射到current_status的json字符串
        MapPetriDish2Json(current_status);
        graph_model.current_status = current_status.dump();
        co_yield graph_model;
        current_status = json::parse(graph_model.current_status);
        MapJson2PetriDish(current_status);
        Update();
    }
}
void CellAutomataFrame_0::Initialize() {
    // 使用随机数引擎随机选择一个细胞位置
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);

    int x = dis(gen);
    int y = dis(gen);

    PetriDish[x][y] = true;
}
void CellAutomataFrame_0::Update() {
    bool newPetriDish[10][10] = { false };

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            int neighbors = CountNeighbors(i, j);

            if (PetriDish[i][j]) {
                // 存活：2 或 3 个邻居
                if (neighbors == 2 || neighbors == 3) {
                    newPetriDish[i][j] = true;
                }
                // 死亡：孤独或拥挤
                else {
                    newPetriDish[i][j] = false;
                }
            }
            else {
                // 繁殖：恰好 3 个邻居
                if (neighbors == 3) {
                    newPetriDish[i][j] = true;
                }
            }
        }
    }

    std::copy(&newPetriDish[0][0], &newPetriDish[0][0] + 10 * 10, &PetriDish[0][0]);
}
int CellAutomataFrame_0::CountNeighbors(int x, int y) const {
    int count = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) continue;
            int nx = x + i;
            int ny = y + j;
            if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10) {
                count += PetriDish[nx][ny];
            }
        }
    }
    return count;
}
void CellAutomataFrame_0::MapPetriDish2Json(json& current_status) const
{
    for(int i = 0; i < 10; ++i)
        for (int j = 0; j < 10; ++j)
        {
            if (PetriDish[i][j] == true)
            {
                json point;
                point["x"] = i;
                point["y"] = j;
                point["z"] = 0;
                point["r"] = 0.5f;
                point["g"] = 0.5f;
                point["b"] = 0.5f;
                point["a"] = 1.0f;
                point["tag"] = i * i + j * j;
                point["blockSize"] = 1;
                current_status.push_back(std::move(point));
            }
            else
            {
                json point;
                point["x"] = i;
                point["y"] = j;
                point["z"] = 0;
                point["r"] = 0;
                point["g"] = 0;
                point["b"] = 0;
                point["a"] = 0.0f;
                point["tag"] = i * i + j * j;
                point["blockSize"] = 1;
                current_status.push_back(std::move(point));
            }
        }
}
void CellAutomataFrame_0::MapJson2PetriDish(const json& current_status)
{
    for (const auto& point : current_status)
    {
        double a = point["a"];
        if (a == 0.0f)
            continue;
        int x = point["x"];
        int y = point["y"];
        double r = point["r"];
        double g = point["g"];
        double b = point["b"];

        int r_val = static_cast<int>(std::round(r * 7));
        int g_val = static_cast<int>(std::round(g * 7));
        int b_val = static_cast<int>(std::round(b * 7));

        // 根据r, g, b解码，更新培养皿
        bool neighbors[9] = { false,false,false,false,true,false,false,false,false };

        neighbors[2] = (r_val >> 2) & 1;
        neighbors[1] = (r_val >> 1) & 1;
        neighbors[0] = (r_val & 1);

        neighbors[5] = (g_val >> 2) & 1;
        neighbors[4] = (g_val >> 1) & 1;
        neighbors[3] = (g_val & 1);

        neighbors[8] = (b_val >> 2) & 1;
        neighbors[7] = (b_val >> 1) & 1;
        neighbors[6] = b_val & 1;

        // 更新周围8个位置的状态
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                int row = x + i;
                int column = y + j;
                if (column >= 0 && column < 10 && row >= 0 && row < 10) {
                    PetriDish[row][column] |= neighbors[(i + 1) * 3 + (j + 1)];
                }
            }
        }
    }
}