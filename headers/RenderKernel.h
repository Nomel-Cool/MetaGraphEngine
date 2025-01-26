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
/// Todo: ���Լ̳�ͬһ��ʵ����FillUp��״̬��״̬���ĸ���
/// ��Ҫ���ڴ���QML���˵�ͼԪ����
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
    /// ����Э�̾�����ⲿ����������ͨ���������ж����GetValue��ȡ��ǰЭ�̱���ֵ������Resume��yield��ָ�Э�̡�
    /// </summary>
    /// <param name="model_name">����������ؿռ���н�����ģ������</param>
    /// <returns>���������Զ�����Э�̾��</returns>
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
