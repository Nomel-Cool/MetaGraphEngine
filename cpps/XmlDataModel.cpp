#include "XmlDataModel.h"

QStringList XmlDataModelFactory::Request4ModelName()
{
    IndexBarModel index_bar_model;
    std::string model_list_xml_path = "./resources/xmls/componentsConfig/model_index.xml";
    auto bound_func = std::bind(&XmlDataModelFactory::FillUp, this, std::placeholders::_1, std::placeholders::_2);
    bool trans_result = file_manager.TransXml2Class<IndexBarModel>(model_list_xml_path, index_bar_model, bound_func);
    if (trans_result)
    {
        QStringList model_names;
        for (const auto& name : index_bar_model.model_name_array)
            model_names.append(QString::fromStdString(name)); 
        return model_names;
    }
    else
        std::cerr << "Convert XML to Class Failed, Abort calling generating function." << std::endl;
    return QStringList();
}

bool XmlDataModelFactory::FillUp(const std::string& json_string, IndexBarModel& model_index)
{
    json json_obj;
    if (!file_manager.TransStr2JsonObject(json_string, json_obj)) {
        std::cerr << "Failed to parse JSON: " << json_string << std::endl;
        return false;
    }
    switch (current_state) {
    case State::WaitingForModel:
        if (json_obj.contains("model_list"))
            current_state = State::WaitingForModelName;
        break;
    case State::WaitingForModelName:
        if (json_obj.contains("model_name")) {
            current_state = State::ReadingModelName;
            return FillUp(json_string, model_index);
        }
        break;
    case State::ReadingModelName:
        if (json_obj["model_name"].is_array())
            for (int i = 0; i < json_obj["model_name"].size(); ++i)
                if (json_obj["model_name"][i].contains("name"))
                {
                    model_index.model_name_array.emplace_back(json_obj["model_name"][i]["name"]);
                    break;
                }
        current_state = State::WaitingForModelName;
        break;
    default:
        throw std::logic_error("Invalid state");
    }
    return false;
}
