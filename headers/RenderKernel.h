#ifndef RENDER_KERNEL
#define RENDER_KERNEL

#include <QObject>
#include <QString>

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
    explicit GraphFactory(QObject* parent = nullptr) : QObject(parent){}
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

protected:
    bool FillUp(const std::string& json_string, GraphModel& graph_model);
    bool Depart(tinyxml2::XMLDocument& doc, GraphModel& graph_model);
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
    DatabaseManager database_manager;
};

#endif // !RENDER_KERNEL
