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
        render_functions["BresenhamCircle"] = std::bind(&RenderCU::BresenhamCircle, this, std::placeholders::_1);
    }
    std::function<std::string(const SingleAutomata&)> GetFunctor(std::string func_name);
protected:
    /// <summary>
    /// 取delta_x, delta_y为起始与终点x与y分量的绝对值
    /// 递推式：pk = 
    /// delta_y-2delta_x, |K| > 1
    /// delta_x-2delta_y, |K| <= 1
    /// 
    /// pk+1=
    /// pk+2delta_x-2delta_y, pk <= 0
    /// pk-2delta_y         , pk > 0
    /// 
    /// </summary>
    /// <param name="graph_model"></param>
    /// <returns></returns>
    virtual std::string BresenhamLine(const SingleAutomata& graph_model);
    /// <summary>
    /// 递推式：di = sqrt(2 * r * i - i * i)，为了效率取di平方，x往右推进，取八分圆第二部分x<=y为大前提，第二限制x * x < di * di
    /// 每次超过第二限制就让y往下一格
    /// </summary>
    /// <param name="graph_model"></param>
    /// <returns></returns>
    virtual std::string BresenhamCircle(const SingleAutomata& graph_model);
private:
    std::map<std::string, std::function<std::string(const SingleAutomata&)>> render_functions;
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
