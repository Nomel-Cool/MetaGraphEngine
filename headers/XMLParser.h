#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "tinyxml2.h"

using namespace tinyxml2;

using TraverseResult = std::vector<std::string>;
class XmlParser
{
public:
    TraverseResult DeSerialize(std::string xml_path)
	{
		XMLDocument xmlDocument;
		XMLError error = xmlDocument.LoadFile(xml_path.c_str());
        TraverseResult traverse_result;
		if (error == XML_SUCCESS) 
            traverse_result = TraversingXML(&xmlDocument);
        return traverse_result;
	}

	bool Serialize(std::string xml_path)
	{

	}
protected:
	/// <summary>
	/// 遍历时只关心xml文档中的 <tag attribute="value1">text</tag> 结构
	/// 用散列表保存等价的Json结构视图：{ "tag" : [ { "attribute" : value } ] }
	/// </summary>
	/// <param name="node">用于遍历XML文件的节点，模型是DOM，从唯一的根节点开始</param>
    TraverseResult TraversingXML(XMLNode* node)
    {
        TraverseResult traverse_result;
        std::stringstream json_stream;

        if (node == nullptr)
            return traverse_result;

        if (node->ToElement())
        {
            auto element = dynamic_cast<XMLElement*>(node);
            json_stream << "{ \"" << element->Name() << "\" : [ ";

            const XMLAttribute* attribute = element->FirstAttribute();
            while (attribute != nullptr)
            {
                json_stream << "{ \"" << attribute->Name() << "\" : \"" << attribute->Value() << "\" }, ";
                attribute = attribute->Next();
            }

            if (node->ToText())
            {
                auto text = dynamic_cast<XMLText*>(node);
                json_stream << "{ \"_text_\" : \"" << text->Value() << "\" } ";
            }

            // Remove the trailing comma and space if they exist
            std::string json_string = json_stream.str();
            if (json_string.back() == ' ')
            {
                json_string.pop_back();
                if (json_string.back() == ',')
                {
                    json_string.pop_back();
                }
            }

            json_string += " ] }";
            traverse_result.push_back(json_string);
            json_stream.str("");  // Clear the stringstream for the next element
        }

        if (!node->NoChildren())
        {
            XMLNode* child = node->FirstChild();
            while (child != nullptr)
            {
                TraverseResult child_result = TraversingXML(child);
                traverse_result.insert(traverse_result.end(), child_result.begin(), child_result.end());
                child = child->NextSibling();
            }
        }

        return traverse_result;
    }

private:

};

#endif // !XML_PARSER_H
