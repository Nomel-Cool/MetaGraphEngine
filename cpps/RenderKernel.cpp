#include "RenderKernel.h"

RenderCU::RenderCU()
{
    render_functions["BresenhamLine"] = std::bind(&RenderCU::BresenhamLine, this, std::placeholders::_1);
    render_functions["BresenhamEllipse"] = std::bind(&RenderCU::BresenhamEllipse, this, std::placeholders::_1);
    render_functions["PartitionBezierCurve"] = std::bind(&RenderCU::PartitionBezierCurve, this, std::placeholders::_1);

    render_co_functions["CoBresenhamLine"] = std::bind(&RenderCU::CoBresenhamLine, this, std::placeholders::_1);
    render_co_functions["CoJustAPoint"] = std::bind(&RenderCU::CoJustAPoint, this, std::placeholders::_1);
}

QString GraphFactory::Request4Model(const QString& model_name)
{
    redis_client.SelectDB(0);
    std::string model_data = redis_client.Get(model_name.toStdString());
    if (model_data.empty()) // 若Redis 没有点阵数据，则从数据库找蓝图文件位置
    {
         GraphModel graph_model;
         std::string automata_xml_path = database_manager.QueryFilePathByModelName(model_name.toStdString());
         /*std::string automata_xml_path = "./resources/xmls/graphAutomata/bezier_automata.xml";*/
         if (automata_xml_path.empty()) // 若数据库中没有蓝图文件的位置，则尝试重建它
             automata_xml_path = database_manager.RebuildXmlFileByModelName(model_name.toStdString());
         auto bound_func = std::bind(&GraphFactory::FillUp, this, std::placeholders::_1, std::placeholders::_2);
         bool trans_result = file_manager.TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
         if(trans_result)
         {
             std::string str_points_list = "[";
             for(const SingleAutomata& sa : graph_model.automatas)
                 str_points_list += render_cu.GetFunctor(sa.func_name)(sa) + ",";
             str_points_list.replace(str_points_list.end() - 1, str_points_list.end(),"]");
             redis_client.Set(model_name.toStdString(), str_points_list);
             return QString(str_points_list.c_str());
         }
         else // 否则重建失败，该模型不存在
         {
             std::cerr << "Convert XML to Class Failed, Abort calling generating function." << std::endl;
             return "";
         }
    }
    return QString(model_data.c_str());
}

ModelGenerator<SingleAutomata> GraphFactory::OfferDynamicModel(const QString& model_name)
{
    GraphModel graph_model;
    //std::string automata_xml_path = "./resources/xmls/graphAutomata/point_automata.xml";
    std::string automata_xml_path = database_manager.QueryFilePathByModelName(model_name.toStdString());
    if (automata_xml_path.empty())
        automata_xml_path = database_manager.RebuildXmlFileByModelName(model_name.toStdString());
    auto bound_func = std::bind(&GraphFactory::FillUp, this, std::placeholders::_1, std::placeholders::_2);
    bool trans_result = file_manager.TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
    if (trans_result)
    {
        for (SingleAutomata& sa : graph_model.automatas)
        {
            auto gen = render_cu.GetCoFunctor("Co" + sa.func_name)(sa);
            do
            {
                co_yield gen.GetValue();  // 将值传递给外层生成器
            }
            while (gen.Resume());
        }
    }
    co_return;
}

bool GraphFactory::FillUp(const std::string& json_string, GraphModel& graph_model) {
    json json_obj;
    if (!file_manager.TransStr2JsonObject(json_string, json_obj)) {
        std::cerr << "Failed to parse JSON: " << json_string << std::endl;
        return false;
    }

    switch (current_state) {
    case State::WaitingForModel:
        if (json_obj.contains("model")) {
            current_state = State::ReadingModelName;
            return FillUp(json_string, graph_model); // Re-process this JSON string in the new state
        }
        break;
    case State::ReadingModelName:
        if (json_obj["model"].is_array())
            for (int i = 0; i < json_obj["model"].size(); ++i)
                if (json_obj["model"][i].contains("name"))
                {
                    graph_model.model_name = json_obj["model"][i]["name"];
                    break;
                }
        current_state = State::WaitingForAutomata;
        break;
    case State::WaitingForAutomata:
        if (json_obj.contains("automata")) {
            current_state = State::ReadingAutomataId;
            graph_model.automatas.emplace_back();
            return FillUp(json_string, graph_model); // Re-process this JSON string in the new state
        }
        break;
    case State::ReadingAutomataId:
        if (json_obj["automata"].is_array())
            for (int i = 0; i < json_obj["automata"].size(); ++i)
                if (json_obj["automata"][i].contains("id"))
                {
                    graph_model.automatas.back().id = json_obj["automata"][i]["id"];
                    break;
                }
        current_state = State::ReadingInitStatus;
        break;
    case State::ReadingInitStatus:
        if (json_obj["init"].is_array())
            for (int i = 0; i < json_obj["init"].size(); ++i)
                if (json_obj["init"][i].contains("init_status"))
                {
                    graph_model.automatas.back().init_status = json_obj["init"][i]["init_status"];
                    break;
                }
        current_state = State::ReadingTransferFunction;
        break;
    case State::ReadingTransferFunction:
        if (json_obj["transfer"].is_array())
            for (int i = 0; i < json_obj["transfer"].size(); ++i)
                if (json_obj["transfer"][i].contains("func_name"))
                {
                    graph_model.automatas.back().func_name = json_obj["transfer"][i]["func_name"];
                    break;
                }
        current_state = State::ReadingCurrentInput;
        break;
    case State::ReadingCurrentInput:
        if (json_obj["input"].is_array())
            for (int i = 0; i < json_obj["input"].size(); ++i)
                if (json_obj["input"][i].contains("current_input"))
                {
                    graph_model.automatas.back().current_input = json_obj["input"][i]["current_input"];
                    break;
                }
        current_state = State::ReadingCurrentStatus;
        break;
    case State::ReadingCurrentStatus:
        if (json_obj["current"].is_array())
            for (int i = 0; i < json_obj["current"].size(); ++i)
                if (json_obj["current"][i].contains("current_status"))
                {
                    graph_model.automatas.back().current_status = json_obj["current"][i]["current_status"];
                    break;
                }
        current_state = State::ReadingTerminateStatus;
        break;
    case State::ReadingTerminateStatus:
        if (json_obj["terminate"].is_array())
            for (int i = 0; i < json_obj["terminate"].size(); ++i)
                if (json_obj["terminate"][i].contains("terminate_status"))
                {
                    graph_model.automatas.back().terminate_status = json_obj["terminate"][i]["terminate_status"];
                    break;
                }
        current_state = State::WaitingForAutomata;
        break;
    default:
        throw std::logic_error("Invalid state");
    }
    return true;
}

bool GraphFactory::Depart(GraphModel& graph_model, tinyxml2::XMLDocument& doc)
{
    try
    {
        // 创建根元素
        tinyxml2::XMLElement* root = doc.NewElement("model");
        root->SetAttribute("name", graph_model.model_name.c_str());

        // 遍历 automatas 并构造子元素
        for (const auto& automata_source : graph_model.automatas)
        {
            tinyxml2::XMLElement* automata = doc.NewElement("automata");
            automata->SetAttribute("id", automata_source.id.c_str());

            tinyxml2::XMLElement* initStatus = doc.NewElement("init");
            initStatus->SetAttribute("init_status", automata_source.init_status.c_str());

            tinyxml2::XMLElement* transferFunction = doc.NewElement("transfer");
            transferFunction->SetAttribute("func_name", automata_source.func_name.c_str());

            tinyxml2::XMLElement* currentInput = doc.NewElement("input");
            currentInput->SetAttribute("current_input", automata_source.current_input.c_str());

            tinyxml2::XMLElement* currentStatus = doc.NewElement("current");
            currentStatus->SetAttribute("current_status", automata_source.current_status.c_str());

            tinyxml2::XMLElement* terminateStatus = doc.NewElement("terminate");
            terminateStatus->SetAttribute("terminate_status", automata_source.terminate_status.c_str());

            automata->InsertFirstChild(initStatus);
            automata->InsertAfterChild(initStatus, transferFunction);
            automata->InsertAfterChild(transferFunction, currentInput);
            automata->InsertAfterChild(currentInput, currentStatus);
            automata->InsertAfterChild(currentStatus, terminateStatus);
            automata->InsertEndChild(terminateStatus);
            root->InsertEndChild(automata);
        }

        // **关键步骤：将根元素设置为文档的根元素**
        doc.InsertFirstChild(root);

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in Depart: " << e.what() << std::endl;
        return false;
    }
    return true;
}

std::string RenderCU::BresenhamLine(const SingleAutomata& graph_model)
{
    json init_status, terminate_status;
    if (!file_manager.TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        return "{}";
    }
    if (!file_manager.TransStr2JsonObject(graph_model.terminate_status, terminate_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.terminate_status << std::endl;
        return "{}";
    }
    std::vector<std::pair<float, float>> points;
    float x0 = init_status["x"];
    float y0 = init_status["y"];
    float xn = terminate_status["x"];
    float yn = terminate_status["y"];
    float dx = fabs(xn - x0);
    float dy = fabs(yn - y0);
    float stepX = (x0 < xn) ? 1 : -1;
    float stepY = (y0 < yn) ? 1 : -1;
    float x = x0;
    float y = y0;

    if (dx > dy) // |m| <= 1
    {
        float p = 2 * dy - dx;
        while (stepX > 0 ? x <= xn : x >= xn) {
            points.emplace_back(std::make_pair(x, y));
            if (p >= 0) {
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
        while (stepY > 0 ? y <= yn : y >= yn) {
            points.emplace_back(std::make_pair(x, y));
            if (p >= 0) {
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
    json init_status, current_status, terminate_status;
    if (!file_manager.TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        co_return;
    }
    if (!file_manager.TransStr2JsonObject(graph_model.terminate_status, terminate_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.terminate_status << std::endl;
        co_return;
    }
    if (!file_manager.TransStr2JsonObject(graph_model.current_status, current_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
        co_return;
    }
    if (current_status.empty()) // 执行初始化
    {
        float init_x = init_status["x"];
        float init_y = init_status["y"];
        current_status["x"] = init_x;
        current_status["y"] = init_y;
        graph_model.current_status = current_status.dump();
    }
    float x0, y0, xn, yn, dx, dy, stepX, stepY, x, y;
    loop_mark:
    x0 = current_status["x"];
    y0 = current_status["y"];
    xn = terminate_status["x"];
    yn = terminate_status["y"];
    dx = fabs(xn - x0);
    dy = fabs(yn - y0);
    stepX = (x0 < xn) ? 1 : -1;
    stepY = (y0 < yn) ? 1 : -1;
    x = x0;
    y = y0;

    if (dx > dy) // |m| <= 1
    {
        float p = 2 * dy - dx;
        if (stepX > 0 ? x <= xn : x >= xn)
        {
            if (p >= 0) 
            {
                y += stepY;
                p -= 2 * dx;
            }
            else
            {
                x += stepX;
                p += 2 * dy;
            }
            json point, ph;
            point["x"] = x;
            point["y"] = y;
            ph["p"] = p;
            graph_model.current_status = point.dump();
            graph_model.current_input = ph.dump();
            // 控制权交到视图交互，视图读写当前graph_model的current_status、current_input或terminate_status后
            // 在此处重新唤醒协程，然后算法根据（可能修改了）当前状态计算控制系数以及下一个点
            co_yield graph_model;
            if (!file_manager.TransStr2JsonObject(graph_model.current_status, current_status))
            {
                std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
                co_return;
            }
            if (!file_manager.TransStr2JsonObject(graph_model.terminate_status, terminate_status))
            {
                std::cerr << "Failed to parse JSON: " << graph_model.terminate_status << std::endl;
                co_return;
            }
        }
        goto loop_mark;
    }
    else // |m| > 1
    {
        float p = 2 * dx - dy;
        if (stepY > 0 ? y <= yn : y >= yn)
        {
            if (p >= 0) {
                x += stepX;
                p -= 2 * dy;
            }
            else
            {
                y += stepY;
                p += 2 * dx;
            }
            json point, ph;
            point["x"] = x;
            point["y"] = y;
            ph["p"] = p;
            graph_model.current_status = point.dump();
            graph_model.current_input = ph.dump();
            co_yield graph_model;
            if (!file_manager.TransStr2JsonObject(graph_model.current_status, current_status))
            {
                std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
                co_return;
            }
            if (!file_manager.TransStr2JsonObject(graph_model.terminate_status, terminate_status))
            {
                std::cerr << "Failed to parse JSON: " << graph_model.terminate_status << std::endl;
                co_return;
            }
        }
        goto loop_mark;
    }
    co_return;
}

ModelGenerator<SingleAutomata> RenderCU::CoJustAPoint(SingleAutomata& graph_model)
{
    json init_status, current_status;
    if (!file_manager.TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        co_return;
    }
    if (!file_manager.TransStr2JsonObject(graph_model.current_status, current_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
        co_return;
    }
    if (current_status.empty()) // 执行初始化
    {
        float init_x = init_status["x"];
        float init_y = init_status["y"];
        current_status["x"] = init_x;
        current_status["y"] = init_y;
        graph_model.current_status = current_status.dump();
    }
    while (true)
    {
        co_yield graph_model;
        if (!file_manager.TransStr2JsonObject(graph_model.current_status, current_status))
        {
            std::cerr << "Failed to parse JSON: " << graph_model.current_status << std::endl;
            co_return;
        }
        if (current_status["x"] <= 0)
            break;
    }
}

std::string RenderCU::BresenhamEllipse(const SingleAutomata& graph_model)
{
    json init_status, terminate_status;
    if (!file_manager.TransStr2JsonObject(graph_model.init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_model.init_status << std::endl;
        return "{}";
    }
    if (!file_manager.TransStr2JsonObject(graph_model.terminate_status, terminate_status))
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
    while (x <= a && y >=0) // 必须指定x和y的边界，否则可能因为曲线过于陡峭使得控制系数增长极度缓慢时，造成内外while的死循环
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

std::string RenderCU::PartitionBezierCurve(const SingleAutomata& graph_model)
{
    json init_status;
    if (!file_manager.TransStr2JsonObject(graph_model.init_status, init_status))
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
