#ifndef RENDER_KERNEL
#define RENDER_KERNEL

#include <iostream>
#include <map>
#include <functional>
#include <QObject>
#include <QString>

#include "MetaGraphAutomata.h"

#include "FileManager.h"
#include "RedisClient.h"

class RenderCU
{
public:
    RenderCU()
    {
        render_functions["BresenhamLine"] = std::bind(&RenderCU::BresenhamLine, this, std::placeholders::_1);
    }
    std::function<std::string(const GraphModel&)> GetFunctor(std::string func_name);
protected:
    virtual std::string BresenhamLine(const GraphModel& graph_model);
private:
    std::map<std::string, std::function<std::string(const GraphModel&)>> render_functions;
    FileManager file_manager;
};

/// <summary>
/// Todo: 可以继承同一个实现了FillUp和状态机状态集的父类
/// 主要用于处理QML与后端的图元请求
/// </summary>
class GraphFactory : public QObject
{
    Q_OBJECT
public:
    explicit GraphFactory(QObject* parent = nullptr) : QObject(parent){}
    Q_INVOKABLE QString Request4Model(const QString& model_name);
protected:
    bool FillUp(const std::string& json_string, GraphModel& graph_model);
private:
    enum class State {
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

    RenderCU render_cu;
    FileManager file_manager;
    RedisClient redis_client;
};

#endif // !RENDER_KERNEL
