#pragma once
#include <memory>
#include "MetaGraphAutomata.h"
#include "IXMLStructure.h"
class IModelDataConvertToStreamStrategy
{
public:
    virtual ~IModelDataConvertToStreamStrategy() = default;
    virtual bool Depart(std::shared_ptr<shabby::IXMLDocument> doc, GraphModel& model) = 0;
};

class IStreamConvertToModelDataStrategy
{
public:
    virtual ~IStreamConvertToModelDataStrategy() = default;
    virtual bool FillUp(const std::string& json_string, GraphModel& graph_model) = 0;
};