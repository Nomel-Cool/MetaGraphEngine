#include "RenderKernel.h"

RenderCU::RenderCU()
{
    render_functions["BresenhamLine"] = std::bind(&RenderCU::BresenhamLine, this, std::placeholders::_1);
    render_functions["BresenhamEllipse"] = std::bind(&RenderCU::BresenhamEllipse, this, std::placeholders::_1);
}

QString GraphFactory::Request4Model(const QString& model_name)
{
    redis_client.SelectDB(0);
    std::string model_data = redis_client.Get(model_name.toStdString());
    if (model_data.empty())
    {
         GraphModel graph_model;
         //std::string automata_xml_path = QueryFromDB(model_name)
         std::string automata_xml_path = "./resources/xmls/graphAutomata/ellipse_automata.xml";
         if(automata_xml_path.empty())
             return "";
         auto bound_func = std::bind(&GraphFactory::FillUp, this, std::placeholders::_1, std::placeholders::_2);
         bool trans_result = file_manager.TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
         std::string str_points_list = "[";
         if(trans_result)
         {
             for(const SingleAutomata& sa : graph_model.automatas)
                 str_points_list += render_cu.GetFunctor(sa.func_name)(sa) + ",";
             str_points_list.replace(str_points_list.end() - 1, str_points_list.end(),"]");
             redis_client.Set(model_name.toStdString(), str_points_list);
             return QString(str_points_list.c_str());
         }
         else
             std::cerr << "Convert XML to Class Failed, Abort calling generating function." << std::endl;
    }
    return QString(model_data.c_str());
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
        while (x != xn) {
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
        while (y != yn) {
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

std::function<std::string(const SingleAutomata&)> RenderCU::GetFunctor(std::string func_name)
{
    return render_functions[func_name];
}