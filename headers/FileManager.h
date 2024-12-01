#pragma once
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <filesystem>
#include <string>
#include <functional>
#include <iostream>

#include "XMLParser.h"
#include "JSONParser.h"

class FileManager
{
public:
    FileManager() {}
    JsonParser GetJsonParser()
    {
        return json_parser;
    }

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

private:
    XmlParser xml_parser;
    JsonParser json_parser;
};

#endif
