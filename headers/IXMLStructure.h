#pragma once
#ifndef I_XML_STRUCTURE_H
#define I_XML_STRUCTURE_H
namespace shabby
{
	class IXMLAttribute
	{
	public:
		virtual ~IXMLAttribute() = default;
		virtual const char* Name() const = 0;
		virtual const char* Value() const = 0;
		virtual std::unique_ptr<IXMLAttribute> Next() const = 0;
	};
	class IXMLText
	{
	public:
		virtual ~IXMLText() = default;
		virtual const char* Value() const = 0;
	};

	class IXMLDocument;
	class IXMLNode
	{
	public:
		virtual ~IXMLNode() = default;
		virtual const char* Name() const = 0;
		virtual std::unique_ptr<IXMLNode> FirstChild() = 0;
		virtual std::unique_ptr<IXMLNode> NextSibling() = 0;
		virtual std::unique_ptr<IXMLNode> InsertFirstChild(std::unique_ptr<IXMLNode> first_node) = 0;
		virtual std::unique_ptr<IXMLNode> InsertAfterChild(std::unique_ptr<IXMLNode> former_node, std::unique_ptr<IXMLNode> later_node) = 0;
		virtual void InsertEndChild(std::unique_ptr<IXMLNode> last_node) = 0;
		virtual std::unique_ptr<IXMLNode> NewNode(std::shared_ptr<IXMLDocument> doc, const char* node_name) = 0;
		virtual std::unique_ptr<IXMLAttribute> FirstAttribute() = 0;
		virtual std::unique_ptr<IXMLText> GetText() = 0;
		virtual void SetAttribute(const char* key, const char* value) = 0;
		virtual bool NoChildren() const = 0;
	};
	class IXMLDocument
	{
	public:
		virtual ~IXMLDocument() = default;
		virtual bool LoadFile(const char* path) = 0;
		virtual bool SaveFile(const char* path) = 0;
		virtual std::unique_ptr<IXMLNode> NewRoot(const char* node_name) = 0;
		virtual std::unique_ptr<IXMLNode> GetRoot() = 0;
	};
	class IXMLDocumentFactory
	{
	public:
		virtual ~IXMLDocumentFactory() = default;
		virtual std::unique_ptr<IXMLDocument> CreateDocument() = 0;
	};
}
#endif // !I_XML_STRUCTURE_H
