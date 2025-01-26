#include "RenderKernel.h"
#include <GraphGenerateAlgorithmLib.h>

GraphFactory::GraphFactory(QObject* parent) : QObject(parent)
{
    // 如果使用其它第三方XML解析库修改工程指针即可
    std::unique_ptr<shabby::IXMLDocumentFactory> tinyxml_factory = std::make_unique<TinyXMLDocumentFactory>();
    sp_file_manager = std::make_shared<FileManager>(std::move(tinyxml_factory));
    sp_stream_to_modeldata = std::make_shared<XMLStreamConvertToModelDataStrategy>();
    sp_modeldata_to_stream = std::make_shared<ModelDataConvertToXMLStreamStrategy>();
    std::unique_ptr<RenderAlgorithmFactory> algo_factory = std::make_unique<RenderAlgorithmFactory>();
    algo_factory->RegisterAlgorithm("BresenhamLine", std::make_unique<BresenhamLineRenderer>(sp_file_manager));
    algo_factory->RegisterAlgorithm("JustAPoint", std::make_unique<JustAPointRenderer>(sp_file_manager));
    algo_factory->RegisterAlgorithm("BresenhamEllipse", std::make_unique<BresenhamEllipseRenderer>(sp_file_manager));
    algo_factory->RegisterAlgorithm("PartitionBezierCurve", std::make_unique<PartitionBezierCurveRenderer>(sp_file_manager));
    algo_factory->RegisterCoAlgorithm("CoBresenhamLine", std::make_unique<BresenhamLineRenderer>(sp_file_manager));
    algo_factory->RegisterCoAlgorithm("CoJustAPoint", std::make_unique<JustAPointRenderer>(sp_file_manager));
    sp_render_cu = std::make_shared<RenderCU>(sp_file_manager, std::move(algo_factory));
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
         auto bound_func = [this](const std::string& json_string, GraphModel& model) {
             return sp_stream_to_modeldata->FillUp(json_string, model);
         };
         bool trans_result = sp_file_manager->TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
         if(trans_result)
         {
             std::string str_points_list = "[";
             for (const SingleAutomata& sa : graph_model.automatas)
                 str_points_list += sp_render_cu->Render(sa.func_name, sa) + ",";
             str_points_list.replace(str_points_list.end() - 1, str_points_list.end(),"]");
             redis_client.Set(model_name.toStdString(), str_points_list);
             return QString(str_points_list.c_str());
         }
         else // 若数据库中没有蓝图文件无效，则尝试重建它
         {
             if (automata_xml_path.empty()) 
                 automata_xml_path = database_manager.RebuildXmlFileByModelName(model_name.toStdString());
             auto bound_func = [this](const std::string& json_string, GraphModel& model) {
                 return sp_stream_to_modeldata->FillUp(json_string, model);
             };
             bool trans_result = sp_file_manager->TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
             if (trans_result)
             {
                 std::string str_points_list = "[";
                 for (const SingleAutomata& sa : graph_model.automatas)
                     str_points_list += sp_render_cu->Render(sa.func_name, sa) + ",";
                 str_points_list.replace(str_points_list.end() - 1, str_points_list.end(), "]");
                 redis_client.Set(model_name.toStdString(), str_points_list);
                 return QString(str_points_list.c_str());
             }
         }
         // 重建失败，则认为该模型损坏，从图元索引中删除它
         std::cerr << "Convert XML to Class Failed, Abort calling generating function." << std::endl;
         return "";
    }
    return QString(model_data.c_str());
}

void GraphFactory::Registry4Model(const QString& model_name, const QString& str_model_data)
{
    // 1. 首先把json数据转换为一个类
    GraphModel graph_model;
    graph_model.model_name = model_name.toStdString();
    json raw_model_data;
    if (!sp_file_manager->TransStr2JsonObject(str_model_data.toStdString(), raw_model_data))
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
    //2. 把装填好的类写入文件
    std::string default_model_path = "./resources/xmls/graphAutomata/" + model_name.toStdString() + ".xml";
    auto bound_func = [this](std::shared_ptr<shabby::IXMLDocument> doc, GraphModel& model) {
        return sp_modeldata_to_stream->Depart(doc, model);
    };
    bool file_trans_result = sp_file_manager->TransClass2Xml<GraphModel>(graph_model, default_model_path, bound_func);
    if (!file_trans_result)
    {
        std::cerr << "Failed to write class to XML file: " << default_model_path << std::endl;
        return;
    }

    //3. 把模型名与文件路径写进数据库
    bool registration_result = database_manager.RegistryModelIndex2DB(model_name.toStdString(), default_model_path);
    if (!registration_result)
    {
        std::cerr << "Failed to insert model NAME into database.";
        return;
    }

    //4. 把模型写入自动机数据库表
    bool record_result = database_manager.RecordModelPieces2DB(graph_model);
    if (!record_result)
    {
        std::cerr << "Failed to insert model DATA into database.";
        return;
    }

    //5.发送信号更新视图索引
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
    auto bound_func = [this](const std::string& json_string, GraphModel& model) {
        return sp_stream_to_modeldata->FillUp(json_string, model);
    };
    bool trans_result = sp_file_manager->TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
    if (trans_result)
    {
        for (SingleAutomata& sa : graph_model.automatas)
        {
            auto gen = sp_render_cu->CoRender("Co" + sa.func_name, sa);
            do
            {
                co_yield gen.GetValue();  // 将值传递给外层生成器
            }
            while (gen.Resume());
        }
    }
    else
    {
        automata_xml_path = database_manager.RebuildXmlFileByModelName(model_name.toStdString());
        auto bound_func = [this](const std::string& json_string, GraphModel& model) {
            return sp_stream_to_modeldata->FillUp(json_string, model);
        };
        bool trans_result = sp_file_manager->TransXml2Class<GraphModel>(automata_xml_path, graph_model, bound_func);
        if (trans_result)
        {
            for (SingleAutomata& sa : graph_model.automatas)
            {
                auto gen = sp_render_cu->CoRender("Co" + sa.func_name, sa);
                do
                {
                    co_yield gen.GetValue();  // 将值传递给外层生成器
                } while (gen.Resume());
            }
        }
    }
    co_return;
}