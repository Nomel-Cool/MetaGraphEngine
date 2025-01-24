#include "ModelDataConvertHelper.h"

bool XMLStreamConvertToModelDataStrategy::FillUp(const std::string& json_string, GraphModel& graph_model)
{
    json json_obj;
    if (!sp_file_manager->TransStr2JsonObject(json_string, json_obj)) {
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

bool ModelDataConvertToXMLStreamStrategy::Depart(std::shared_ptr<shabby::IXMLDocument> doc, GraphModel& graph_model)
{
    try
    {
        // 创建根元素
        std::unique_ptr<shabby::IXMLNode> root = doc->NewRoot("model");
        root->SetAttribute("name", graph_model.model_name.c_str());

        // 遍历 automatas 并构造子元素
        for (const auto& automata_source : graph_model.automatas)
        {
            std::unique_ptr<shabby::IXMLNode> automata = root->NewNode(doc, "automata");
            automata->SetAttribute("id", automata_source.id.c_str());

            // 自定义属性值转义函数
            auto escapeAttribute = [](const std::string& input) -> std::string {
                std::string escaped;
                for (char c : input)
                {
                    switch (c)
                    {
                    case '\"': escaped += "\\\""; break; // 转义双引号
                    case '\'': escaped += "\\\'"; break; // 转义单引号
                    case '&': escaped += "&amp;"; break; // 转义 & 为 &amp;
                    case '<': escaped += "&lt;"; break;  // 转义 < 为 &lt;
                    case '>': escaped += "&gt;"; break;  // 转义 > 为 &gt;
                    default: escaped += c; break;        // 其他字符不变
                    }
                }
                return escaped;
                };

            // 设置子元素及其属性
            std::unique_ptr<shabby::IXMLNode> initStatus = root->NewNode(doc, "init");
            initStatus->SetAttribute("init_status", escapeAttribute(automata_source.init_status).c_str());

            std::unique_ptr<shabby::IXMLNode> transferFunction = root->NewNode(doc, "transfer");
            transferFunction->SetAttribute("func_name", automata_source.func_name.c_str());

            std::unique_ptr<shabby::IXMLNode> currentInput = root->NewNode(doc, "input");
            currentInput->SetAttribute("current_input", escapeAttribute(automata_source.current_input).c_str());

            std::unique_ptr<shabby::IXMLNode> currentStatus = root->NewNode(doc, "current");
            currentStatus->SetAttribute("current_status", escapeAttribute(automata_source.current_status).c_str());

            std::unique_ptr<shabby::IXMLNode> terminateStatus = root->NewNode(doc, "terminate");
            terminateStatus->SetAttribute("terminate_status", escapeAttribute(automata_source.terminate_status).c_str());

            automata->InsertFirstChild(std::move(initStatus));
            transferFunction = std::move(automata->InsertAfterChild(std::move(initStatus), std::move(transferFunction)));
            currentInput = std::move(automata->InsertAfterChild(std::move(transferFunction), std::move(currentInput)));
            currentStatus = std::move(automata->InsertAfterChild(std::move(currentInput), std::move(currentStatus)));
            automata->InsertAfterChild(std::move(currentStatus), std::move(terminateStatus));
            root->InsertEndChild(std::move(automata));
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in Depart: " << e.what() << std::endl;
        return false;
    }
    return true;
}