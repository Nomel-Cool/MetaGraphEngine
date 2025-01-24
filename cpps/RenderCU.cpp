#include "RenderCU.h"

RenderCU::RenderCU()
{
    render_functions["BresenhamLine"] = std::bind(&RenderCU::BresenhamLine, this, std::placeholders::_1);
    render_functions["BresenhamEllipse"] = std::bind(&RenderCU::BresenhamEllipse, this, std::placeholders::_1);
    render_functions["PartitionBezierCurve"] = std::bind(&RenderCU::PartitionBezierCurve, this, std::placeholders::_1);

    render_co_functions["CoBresenhamLine"] = std::bind(&RenderCU::CoBresenhamLine, this, std::placeholders::_1);
    render_co_functions["CoJustAPoint"] = std::bind(&RenderCU::CoJustAPoint, this, std::placeholders::_1);

    // ���ʹ������������XML�������޸Ĺ���ָ�뼴��
    std::unique_ptr<shabby::IXMLDocumentFactory> tinyxml_factory = std::make_unique<TinyXMLDocumentFactory>();
    sp_file_manager = std::make_shared<FileManager>(std::move(tinyxml_factory));
}

std::string RenderCU::JustAPoint(const SingleAutomata& graph_model)
{
    json init_status;
    if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        return "{}";
    }
    return init_status.dump();
}

ModelGenerator<SingleAutomata> RenderCU::CoJustAPoint(SingleAutomata& graph_model)
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
    // ����Э�飬�����Զ����ֶζ�������
    if (current_status.empty()) // ִ�г�ʼ��
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

std::string RenderCU::BresenhamLine(const SingleAutomata& graph_model)
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

ModelGenerator<SingleAutomata> RenderCU::CoBresenhamLine(SingleAutomata& graph_model)
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


std::string RenderCU::BresenhamEllipse(const SingleAutomata& graph_model)
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

    // ������Ҫ����Ч����ȥ��points_part_1~points_part_8
    std::vector<std::pair<float, float>> points_part;
    std::vector<std::pair<float, float>> points_part_1;
    std::vector<std::pair<float, float>> points_part_2;
    std::vector<std::pair<float, float>> points_part_3;
    std::vector<std::pair<float, float>> points_part_4;

    // ������Ҫ����Ч������������points_part_1~points_part_8Ϊpoints_part���ɡ�
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
    float index = 1; // �����Ϊ0�����ʼ��ᱻ���룬����һ���ظ�������
    while (x <= a && y >= 0) // ����ָ��x��y�ı߽磬���������Ϊ���߹��ڶ���ʹ�ÿ���ϵ���������Ȼ���ʱ���������while����ѭ��
    {
        dd = 2 * b * index * a * a - index * index * a * a;
        while (b * b * x * x <= dd) // ȡ <= ��ƫ��ȡƽ����x�ᣬȡ < ��ƫ��ȡƽ��y��
        {
            add_circle_points(xc, yc, x, y);
            x++;
        }
        add_circle_points(xc, yc, x, y - 1); // ��y�������߹��ڶ��͵�ʱ��һ��x���ܿ������½���־�����Ծ���ÿ��x����һ���½���־����Ҫֱ���½���
        index++;
        y--;
    }

    // ������Ҫ����Ч����ɾ����������insert
    points_part.insert(points_part.end(), points_part_4.rbegin(), points_part_4.rend());
    points_part.insert(points_part.end(), points_part_3.begin(), points_part_3.end());
    points_part.insert(points_part.end(), points_part_2.rbegin(), points_part_2.rend());
    points_part.insert(points_part.end(), points_part_1.begin(), points_part_1.end());

    json result;
    for (const auto& point : points_part)
        result.push_back({ {"point", {point.first, point.second}} });

    return result.dump();
}

std::string RenderCU::PartitionBezierCurve(const SingleAutomata& graph_model)
{
    json init_status;
    if (!sp_file_manager->TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        return "{}";
    }

    int n = init_status.size() - 1; // ���������ߵĽ���Ϊ���Ƶ��� - 1

    // ���������߷���
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

    // ���������������
    float left = init_status[0]["x"];
    float right = init_status[0]["x"];
    for (const auto& p : init_status) {
        if (p["x"] < left) left = p["x"];
        if (p["x"] > right) right = p["x"];
    }

    std::vector<std::pair<float, float>> points_part;

    // ƽ���ֳ�100��
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

int RenderCU::binomial_coeff(int n, int k)
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

std::function<std::string(const SingleAutomata&)> RenderCU::GetFunctor(std::string func_name)
{
    return render_functions[func_name];
}

std::function<ModelGenerator<SingleAutomata>(SingleAutomata&)> RenderCU::GetCoFunctor(std::string func_name)
{
    return render_co_functions[func_name];
}
