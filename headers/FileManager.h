#pragma once

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <filesystem>
#include <string>
#include <functional>
#include <iostream>

#include "XMLParser.h"
#include "JSONParser.h"

class  FileManager
{
public:
    FileManager() {}
    bool TransStr2JsonObject(const std::string& json_string, json& json_obj);

    template<typename StructuringClass>
    bool TransXml2Class(const std::string& xml_path, StructuringClass& target_obj, std::function<bool(const std::string&, StructuringClass&)> FillUp)
    {
        auto deserialized_data = xml_parser.DeSerialize(xml_path);
        if (!deserialized_data.empty())
        {
            try {
                for (const auto& json_string : deserialized_data)
                    FillUp(json_string, target_obj);
                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "Exception occurred: " << e.what() << std::endl;
                return false;
            }
        }
        else
        {
            std::cerr << "DeSerialization failed or returned empty data." << std::endl;
            return false;
        }
    }

    template<typename StructuringClass>
    bool TransClass2Xml(StructuringClass& target_obj, const std::string& xml_path, const std::function<bool(tinyxml2::XMLDocument&, StructuringClass&)>& Depart)
    {
        try {
            // 创建一个空的 XML 文档
            tinyxml2::XMLDocument doc;

            // 通过 Depart 回调函数填充 XML 文档
            if (!Depart(doc, target_obj)) {
                std::cerr << "Depart function failed to generate XML data." << std::endl;
                return false;
            }

            // 调用 Serialize 保存到指定路径
            if (!xml_parser.Serialize(xml_path, doc)) {
                std::cerr << "Failed to serialize XML document to path: " << xml_path << std::endl;
                return false;
            }

            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception occurred during TransClass2Xml: " << e.what() << std::endl;
            return false;
        }
    }

private:
    XmlParser xml_parser;
    JsonParser json_parser;
};

#endif
