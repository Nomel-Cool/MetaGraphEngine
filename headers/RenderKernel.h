#ifndef RENDER_KERNEL
#define RENDER_KERNEL

#include <iostream>
#include <map>
#include <functional>
#include <cmath>
#include <algorithm>
#include <QObject>
#include <QString>

#include "MetaGraphAutomata.h"
#include "CoModelGenerator.h"

#include "FileManager.h"
#include "RedisClient.h"

// 当像素控件调用生成算法时，默认是查找现有指定算法的Dynamic前缀的动态算法，所以像素控件和预览板可以读取同一份xml。
// 预览板使用的是静态一次性生成点集，而像素控件中每个点的生成都是可影响的。

class RenderCU
{
public:
    RenderCU();
    std::function<std::string(const SingleAutomata&)> GetFunctor(std::string func_name);
    std::function<ModelGenerator<SingleAutomata>(SingleAutomata&)> GetCoFunctor(std::string func_name);
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
    virtual ModelGenerator<SingleAutomata> CoBresenhamLine(SingleAutomata& graph_model);
    virtual ModelGenerator<SingleAutomata> CoJustAPoint(SingleAutomata& graph_model);
    /// <summary>
    /// 递推式：di = sqrt(2 * r * i - i * i) * a / b，为了效率取di平方再乘b的平方，x往右推进，取八分圆第二部分x<=y为大前提，第二限制x * x < di * di
    /// 每次超过第二限制就让y往下一格
    /// </summary>
    /// <param name="graph_model"></param>
    /// <returns></returns>
    virtual std::string BresenhamEllipse(const SingleAutomata& graph_model);

    virtual std::string PartitionBezierCurve(const SingleAutomata& graph_model);

    // 计算组合数C(n, k)
    int binomial_coeff(int n, int k);
private:
    std::map<std::string, std::function<std::string(const SingleAutomata&)>> render_functions;
    std::map<std::string, std::function<ModelGenerator<SingleAutomata>(SingleAutomata&)>> render_co_functions;
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
    /// <summary>
    /// 传递协程句柄到外部交互环境，通过生成器中定义的GetValue获取当前协程变量值，或者Resume从yield点恢复协程。
    /// </summary>
    /// <param name="model_name">请求加入像素空间进行交互的模型名称</param>
    /// <returns>包含单体自动机的协程句柄</returns>
    ModelGenerator<SingleAutomata> OfferDynamicModel(const QString& model_name);
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
