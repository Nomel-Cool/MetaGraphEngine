#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "TinyXMLImpl.h"

class XMLTraverser
{
public:
	/// <summary>
	/// 遍历时只关心xml文档中的 <tag attribute="value1">text</tag> 结构
	/// 用散列表保存等价的Json结构视图：{ "tag" : [ { "attribute" : value } ] }
	/// </summary>
	/// <param name="node">用于遍历XML文件的节点，模型是DOM，从唯一的根节点开始</param>
	std::vector<std::string> TraversingXML(shabby::IXMLNode& node);
};

class XMLSerializer
{
public:
	/// <summary>
	/// 将结构化对象的XML内容持久化到指定文件
	/// </summary>
	/// <param name="xml_path">文件目录</param>
	/// <param name="doc">结构化的XML对象</param>
	/// <returns>成功返回true，否则false</returns>
	bool Serialize(const std::string& xml_path, shabby::IXMLDocument& doc);
};

class XMLDeserializer
{
public:
	XMLDeserializer(std::unique_ptr<shabby::IXMLDocumentFactory> factory);
	/// <summary>
	/// 加载给定的 XML 文件 URL
	/// </summary>
	/// <param name="xml_path"> xml 文件路径 </param>
	/// <returns>成功则返回文件指针，否则空指针</returns>
	std::unique_ptr<shabby::IXMLDocument> DeSerialize(std::string xml_path);
private:
	std::unique_ptr<shabby::IXMLDocumentFactory> factory_;
};

class XmlParser
{
public:
	XmlParser(std::unique_ptr<shabby::IXMLDocumentFactory> factory);
	bool DoSerialization(const std::string& xml_path, shabby::IXMLDocument& doc);
	std::vector<std::string> DoDeserialization(const std::string& xml_path);
private:
	std::unique_ptr<XMLTraverser> up_xml_traverser;
	std::unique_ptr<XMLSerializer> up_xml_serializer;
	std::unique_ptr<XMLDeserializer> up_xml_deserializer;
};

#endif // !XML_PARSER_H
