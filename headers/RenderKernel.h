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
    /// ȡdelta_x, delta_yΪ��ʼ���յ�x��y�����ľ���ֵ
    /// ����ʽ��pk = 
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
    /// ����ʽ��di = sqrt(2 * r * i - i * i)��Ϊ��Ч��ȡdiƽ����x�����ƽ���ȡ�˷�Բ�ڶ�����x<=yΪ��ǰ�ᣬ�ڶ�����x * x < di * di
    /// ÿ�γ����ڶ����ƾ���y����һ��
    /// </summary>
    /// <param name="graph_model"></param>
    /// <returns></returns>
    virtual std::string BresenhamCircle(const SingleAutomata& graph_model);
private:
    std::map<std::string, std::function<std::string(const SingleAutomata&)>> render_functions;
    FileManager file_manager;
};

/// <summary>
/// Todo: ���Լ̳�ͬһ��ʵ����FillUp��״̬��״̬���ĸ���
/// ��Ҫ���ڴ���QML���˵�ͼԪ����
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
