#pragma once
#ifndef TINYXML_IMPL_H
#define TINYXML_IMPL_H

#include <memory>

#include "IXMLStructure.h"

#include "tinyxml2.h"

class TinyXMLAttributeAdapter : public shabby::IXMLAttribute
{
public:
	TinyXMLAttributeAdapter(const tinyxml2::XMLAttribute* init_attribute);
	const char* Name() const override;
	const char* Value() const override;
	std::unique_ptr<shabby::IXMLAttribute> Next() const override;
private:
	tinyxml2::XMLAttribute* attribute;
};

class TinyXMLTextAdapter : public shabby::IXMLText
{
public:
	TinyXMLTextAdapter(const tinyxml2::XMLText* init_text);
	const char* Value() const override;
private:
	tinyxml2::XMLText* text;
};

class TinyXMLNodeAdapter : public shabby::IXMLNode
{
public:
	TinyXMLNodeAdapter(const tinyxml2::XMLNode* init_node);
	const char* Name() const override;
	std::unique_ptr<shabby::IXMLNode> FirstChild() override;
	std::unique_ptr<shabby::IXMLNode> NextSibling() override;
	std::unique_ptr<shabby::IXMLNode> InsertFirstChild(std::unique_ptr<shabby::IXMLNode> first_node) override;
	std::unique_ptr<shabby::IXMLNode> InsertAfterChild(std::unique_ptr<shabby::IXMLNode> former_node, std::unique_ptr<shabby::IXMLNode> later_node) override;
	void InsertEndChild(std::unique_ptr<shabby::IXMLNode> last_node) override;
	std::unique_ptr<shabby::IXMLNode> NewNode(std::shared_ptr<shabby::IXMLDocument> doc, const char* node_name) override;
	void SetAttribute(const char* key, const char* value) override;
	std::unique_ptr<shabby::IXMLAttribute> FirstAttribute() override;
	std::unique_ptr<shabby::IXMLText> GetText() override;
	bool NoChildren() const override;
private:
	tinyxml2::XMLNode* node;
};


class TinyXMLDocumentAdapter : public shabby::IXMLDocument
{
public:
	TinyXMLDocumentAdapter();
	TinyXMLDocumentAdapter(const tinyxml2::XMLDocument* init_doc);
	~TinyXMLDocumentAdapter() override;
	bool LoadFile(const char* path) override;
	bool SaveFile(const char* path) override;
	std::unique_ptr<shabby::IXMLNode> NewRoot(const char* node_name) override;
	std::unique_ptr<shabby::IXMLNode> GetRoot() override;
	tinyxml2::XMLDocument* GetDocHandler();
private:
	tinyxml2::XMLDocument* doc;
};
class TinyXMLDocumentFactory : public shabby::IXMLDocumentFactory 
{
public:
	std::unique_ptr<shabby::IXMLDocument> CreateDocument() override;
};
#endif // !TINYXML_IMPL_H
