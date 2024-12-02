#include "RenderKernel.h"

void GraphFactory::CallFromIndexBar(const QString& func_name)
{
    GraphModel graph_model;
    std::string file_name = "resources/xmls/graphAutomata/segment_automata.xml";
    auto bound_func = std::bind(&GraphFactory::FillUp, this, std::placeholders::_1, std::placeholders::_2);
    bool trans_result = file_manager.TransXml2Class<GraphModel>(file_name, graph_model, bound_func);
    if (trans_result)
        std::string str_points_list = render_cu.GetFunctor(func_name.toStdString())(graph_model);
    else
        std::cerr << "Convert XML to Class Failed, Abort calling generating function." << std::endl;
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

std::string RenderCU::BresenhamLine(const GraphModel& graph_model)
{
    json init_status, terminate_status;
    auto graph_automata = graph_model.automatas.begin();
    if (!file_manager.TransStr2JsonObject(graph_automata->init_status, init_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_automata->init_status << std::endl;
        return "{}";
    }
    if (!file_manager.TransStr2JsonObject(graph_automata->terminate_status, terminate_status))
    {
        std::cerr << "Failed to parse JSON: " << graph_automata->terminate_status << std::endl;
        return "{}";
    }
    std::vector<std::pair<float, float>> points;
    float x0 = init_status["x"];
    float y0 = init_status["y"];
    float xn = terminate_status["x"];
    float yn = terminate_status["y"];
    float x = x0;
    float y = y0;
    float dx = abs(xn - x0);
    float dy = abs(yn - y0);
    float stepX = dx >= 0 ? 1 : -1;
    float stepY = dy >= 0 ? 1 : -1;
    float p = dx < dy ? dy - 2 * dx : dx - 2 * dy;
    if (dx > dy) // |m| <= 1
    {
        float p = dx - 2 * dy;
        float y = y0;
        for (float x = x0; x <= xn; x += stepX)
        {
            points.emplace_back(std::make_pair(x, y));
            if (p <= 0)
            {
                y += stepY;
                p += 2 * (dy - dx);
            }
            p -= 2 * dy;
        }
    }
    else // |m| > 1
    {
        float p = dy - 2 * dx;
        float x = x0;
        for (float y = y0; y <= yn; y += stepY)
        {
            points.emplace_back(std::make_pair(x, y));
            if (p <= 0)
            {
                x += stepX;
                p += 2 * (dy - dx);
            }
            p -= 2 * dy;
        }
    }
    json result;
    for (const auto& point : points)
    {
        result.push_back({ {"point", {point.first, point.second}} });
    }
    return result.dump();
}

std::function<std::string(const GraphModel&)> RenderCU::GetFunctor(std::string func_name)
{
    return render_functions[func_name];
}