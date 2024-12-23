#include "RenderKernel.h"

QString GraphFactory::Request4Model(const QString& model_name)
{
    redis_client.SelectDB(0);
    std::string model_data = redis_client.Get(model_name.toStdString());
    if (model_data.empty()) // ��Redis û�е������ݣ�������ݿ�����ͼ�ļ�λ��
    {
         GraphModel graph_model;
         std::string automata_xml_path = database_manager.QueryFilePathByModelName(model_name.toStdString());
         /*std::string automata_xml_path = "./resources/xmls/graphAutomata/bezier_automata.xml";*/
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
         else // �����ݿ���û����ͼ�ļ���Ч�������ؽ���
         {
             if (automata_xml_path.empty()) 
                 automata_xml_path = database_manager.RebuildXmlFileByModelName(model_name.toStdString());
             auto bound_func = std::bind(&GraphFactory::FillUp, this, std::placeholders::_1, std::placeholders::_2);
             bool trans_result = file_manager.TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
             if (trans_result)
             {
                 std::string str_points_list = "[";
                 for (const SingleAutomata& sa : graph_model.automatas)
                     str_points_list += render_cu.GetFunctor(sa.func_name)(sa) + ",";
                 str_points_list.replace(str_points_list.end() - 1, str_points_list.end(), "]");
                 redis_client.Set(model_name.toStdString(), str_points_list);
                 return QString(str_points_list.c_str());
             }
         }
         // �ؽ�ʧ�ܣ�����Ϊ��ģ���𻵣���ͼԪ������ɾ����
         std::cerr << "Convert XML to Class Failed, Abort calling generating function." << std::endl;
         return "";
    }
    return QString(model_data.c_str());
}

void GraphFactory::Registry4Model(const QString& model_name, const QString& str_model_data)
{
    // 1. ���Ȱ�json����ת��Ϊһ����
    GraphModel graph_model;
    graph_model.model_name = model_name.toStdString();
    json raw_model_data;
    if (!file_manager.TransStr2JsonObject(str_model_data.toStdString(), raw_model_data))
    {
        std::cerr << "Registration Failed: The jsonization of " << str_model_data.toStdString() << " is invalid." << std::endl;
        return;
    }
    // [
    //     { "singleAutomata":
    //       [
    //         {"type":"init", "init_status" : ""},
    //         { "type":"transfer","func_name" : "" },
    //         { "type":"input","current_input" : "" },
    //         { "type":"current","current_status" : "" },
    //         { "type":"terminate","terminate_status" : "" }
    //      ]
    //     },
    //     { "singleAutomata":
    //       [
    //         {"type":"init","init_status" : ""},
    //         {"type":"transfer","func_name" : ""},
    //         {"type":"input","current_input" : ""},
    //         {"type":"current","current_status" : ""},
    //         {"type":"terminate","terminate_status" : ""}
    //      ] 
    //    }
    // ]
    if (!raw_model_data.is_array())
    {
        std::cerr << "Expected " << str_model_data.toStdString() << " as a array." << std::endl;
        return;
    }
    for (int i = 0; i < raw_model_data.size(); ++i)
    {
        graph_model.automatas.emplace_back(SingleAutomata());
        if (!raw_model_data[i]["singleAutomata"].is_array())
        {
            std::cerr << "Expected " << raw_model_data[i]["singleAutomata"] << " as a array." << std::endl;
            return;
        }
        for (int j = 0; j < raw_model_data[i]["singleAutomata"].size(); ++j)
        {
            graph_model.automatas[i].id = std::to_string(i + 1);
            if (raw_model_data[i]["singleAutomata"][j]["type"] == "init")
                graph_model.automatas[i].init_status = raw_model_data[i]["singleAutomata"][j]["init_status"];
            if (raw_model_data[i]["singleAutomata"][j]["type"] == "transfer")
                graph_model.automatas[i].func_name = raw_model_data[i]["singleAutomata"][j]["func_name"];
            if (raw_model_data[i]["singleAutomata"][j]["type"] == "input")
                graph_model.automatas[i].current_input = raw_model_data[i]["singleAutomata"][j]["current_input"];
            if (raw_model_data[i]["singleAutomata"][j]["type"] == "current")
                graph_model.automatas[i].current_status = raw_model_data[i]["singleAutomata"][j]["current_status"];
            if (raw_model_data[i]["singleAutomata"][j]["type"] == "terminate")
                graph_model.automatas[i].terminate_status = raw_model_data[i]["singleAutomata"][j]["terminate_status"];
        }
    }
    //2. ��װ��õ���д���ļ�
    std::string default_model_path = "./resources/xmls/graphAutomata/" + model_name.toStdString() + ".xml";
    auto bound_func = std::bind(&GraphFactory::Depart, this, std::placeholders::_1, std::placeholders::_2);
    bool file_trans_result = file_manager.TransClass2Xml<GraphModel>(graph_model, default_model_path, bound_func);
    if (!file_trans_result)
    {
        std::cerr << "Failed to write class to XML file: " << default_model_path << std::endl;
        return;
    }

    //3. ��ģ�������ļ�·��д�����ݿ�
    bool registration_result = database_manager.RegistryModelIndex2DB(model_name.toStdString(), default_model_path);
    if (!registration_result)
    {
        std::cerr << "Failed to insert model NAME into database.";
        return;
    }

    //4. ��ģ��д���Զ������ݿ��
    bool record_result = database_manager.RecordModelPieces2DB(graph_model);
    if (!record_result)
    {
        std::cerr << "Failed to insert model DATA into database.";
        return;
    }

    //5.�����źŸ�����ͼ����
    emit update4NameList();
}

QStringList GraphFactory::Request4ModelNameList() 
{
    QStringList modelNames;

    try {
        auto name_vec = database_manager.GetModelNameList();
        for (const auto& model_name : name_vec)
            modelNames.emplaceBack(QString::fromStdString(model_name));
    }
    catch (const std::exception& e) {
        std::cerr << "Exception occurred:" << e.what() << std::endl;
    }

    return modelNames;
}

ModelGenerator<SingleAutomata> GraphFactory::OfferDynamicModel(const QString& model_name)
{
    GraphModel graph_model;
    //std::string automata_xml_path = "./resources/xmls/graphAutomata/point_automata.xml";
    std::string automata_xml_path = database_manager.QueryFilePathByModelName(model_name.toStdString());
    auto bound_func = std::bind(&GraphFactory::FillUp, this, std::placeholders::_1, std::placeholders::_2);
    bool trans_result = file_manager.TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
    if (trans_result)
    {
        for (SingleAutomata& sa : graph_model.automatas)
        {
            auto gen = render_cu.GetCoFunctor("Co" + sa.func_name)(sa);
            do
            {
                co_yield gen.GetValue();  // ��ֵ���ݸ����������
            }
            while (gen.Resume());
        }
    }
    else
    {
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
                    co_yield gen.GetValue();  // ��ֵ���ݸ����������
                } while (gen.Resume());
            }
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

bool GraphFactory::Depart(tinyxml2::XMLDocument& doc, GraphModel& graph_model)
{
    try
    {
        // ������Ԫ��
        tinyxml2::XMLElement* root = doc.NewElement("model");
        root->SetAttribute("name", graph_model.model_name.c_str());

        // ���� automatas ��������Ԫ��
        for (const auto& automata_source : graph_model.automatas)
        {
            tinyxml2::XMLElement* automata = doc.NewElement("automata");
            automata->SetAttribute("id", automata_source.id.c_str());

            // �Զ�������ֵת�庯��
            auto escapeAttribute = [](const std::string& input) -> std::string {
                std::string escaped;
                for (char c : input)
                {
                    switch (c)
                    {
                    case '\"': escaped += "\\\""; break; // ת��˫����
                    case '\'': escaped += "\\\'"; break; // ת�嵥����
                    case '&': escaped += "&amp;"; break; // ת�� & Ϊ &amp;
                    case '<': escaped += "&lt;"; break;  // ת�� < Ϊ &lt;
                    case '>': escaped += "&gt;"; break;  // ת�� > Ϊ &gt;
                    default: escaped += c; break;        // �����ַ�����
                    }
                }
                return escaped;
                };

            // ������Ԫ�ؼ�������
            tinyxml2::XMLElement* initStatus = doc.NewElement("init");
            initStatus->SetAttribute("init_status", escapeAttribute(automata_source.init_status).c_str());

            tinyxml2::XMLElement* transferFunction = doc.NewElement("transfer");
            transferFunction->SetAttribute("func_name", automata_source.func_name.c_str());

            tinyxml2::XMLElement* currentInput = doc.NewElement("input");
            currentInput->SetAttribute("current_input", escapeAttribute(automata_source.current_input).c_str());

            tinyxml2::XMLElement* currentStatus = doc.NewElement("current");
            currentStatus->SetAttribute("current_status", escapeAttribute(automata_source.current_status).c_str());

            tinyxml2::XMLElement* terminateStatus = doc.NewElement("terminate");
            terminateStatus->SetAttribute("terminate_status", escapeAttribute(automata_source.terminate_status).c_str());

            automata->InsertFirstChild(initStatus);
            automata->InsertAfterChild(initStatus, transferFunction);
            automata->InsertAfterChild(transferFunction, currentInput);
            automata->InsertAfterChild(currentInput, currentStatus);
            automata->InsertAfterChild(currentStatus, terminateStatus);
            root->InsertEndChild(automata);
        }

        // ���ø�Ԫ��
        doc.InsertFirstChild(root);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in Depart: " << e.what() << std::endl;
        return false;
    }
    return true;
}