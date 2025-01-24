#pragma once
#ifndef MODELDATA_CONVERT_HELPER_H
#define MODELDATA_CONVERT_HELPER_H

#include <memory>
#include <string>

#include "IModelDataConverter.h"
#include "FileManager.h"

class XMLStreamConvertToModelDataStrategy : public IStreamConvertToModelDataStrategy
{
public:
    bool FillUp(const std::string& json_string, GraphModel& graph_model) override;

private:
    enum State {
        WaitingForModel,
        ReadingModelName,
        WaitingForAutomata,
        ReadingAutomataId,
        ReadingInitStatus,
        ReadingTransferFunction,
        ReadingCurrentInput,
        ReadingCurrentStatus,
        ReadingTerminateStatus
    };
    State current_state = State::WaitingForModel;
    std::shared_ptr<FileManager> sp_file_manager;
};

class ModelDataConvertToXMLStreamStrategy : public IModelDataConvertToStreamStrategy
{
public:
    bool Depart(std::shared_ptr<shabby::IXMLDocument> doc, GraphModel& model) override;
};

#endif // !MODELDATA_CONVERT_HELPER_H
