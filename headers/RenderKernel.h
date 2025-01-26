#ifndef RENDER_KERNEL
#define RENDER_KERNEL

#include <QObject>
#include <QString>

#include "ModelDataConvertHelper.h"
#include "RenderCU.h"
#include "FileManager.h"
#include "RedisClient.h"
#include "DataBaseManager.h"

/// <summary>
/// Todo: 可以继承同一个实现了FillUp和状态机状态集的父类
/// 主要用于处理QML与后端的图元请求
/// </summary>
class GraphFactory : public QObject
{
    Q_OBJECT
    friend class Law;
public:
    explicit GraphFactory(QObject* parent = nullptr);
    Q_INVOKABLE QString Request4Model(const QString& model_name);
    Q_INVOKABLE void Registry4Model(const QString& model_name, const QString& str_model_data);
    Q_INVOKABLE QStringList Request4ModelNameList();
    /// <summary>
    /// 传递协程句柄到外部交互环境，通过生成器中定义的GetValue获取当前协程变量值，或者Resume从yield点恢复协程。
    /// </summary>
    /// <param name="model_name">请求加入像素空间进行交互的模型名称</param>
    /// <returns>包含单体自动机的协程句柄</returns>
    ModelGenerator<SingleAutomata> OfferDynamicModel(const QString& model_name);

signals:
    QStringList update4NameList();

private:

    std::shared_ptr<IStreamConvertToModelDataStrategy> sp_stream_to_modeldata;
    std::shared_ptr<IModelDataConvertToStreamStrategy> sp_modeldata_to_stream;
    std::shared_ptr<RenderCU> sp_render_cu;
    std::shared_ptr<FileManager> sp_file_manager;
    RedisClient redis_client;
    DatabaseManager database_manager;
};

#endif // !RENDER_KERNEL
