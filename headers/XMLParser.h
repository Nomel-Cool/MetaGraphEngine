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
    TraverseResult DeSerialize(std::string xml_path);

	bool Serialize(const std::string& xml_path, XMLDocument& doc);
protected:
	/// <summary>
	/// 遍历时只关心xml文档中的 <tag attribute="value1">text</tag> 结构
	/// 用散列表保存等价的Json结构视图：{ "tag" : [ { "attribute" : value } ] }
	/// </summary>
	/// <param name="node">用于遍历XML文件的节点，模型是DOM，从唯一的根节点开始</param>
	TraverseResult TraversingXML(XMLNode* node);

private:

};

#endif // !XML_PARSER_H
