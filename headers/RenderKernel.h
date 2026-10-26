#ifndef RENDER_KERNEL
#define RENDER_KERNEL

#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>
#include <mutex>
#include <string>

#include "ModelDataConvertHelper.h"
#include "RenderCU.h"
#include "FileManager.h"
#include "RedisClient.h"
#include "DataBaseManager.h"

/// <summary>
/// Bridge between QML and the model/render storage.
/// </summary>
class GraphFactory : public QObject
{
    Q_OBJECT
    friend class Law;
public:
    explicit GraphFactory(QObject* parent = nullptr);

    Q_INVOKABLE QString Request4Model(const QString& model_name);
    Q_INVOKABLE QString Request4ModelCatalog();
    Q_INVOKABLE QString Request4ModelPreview(const QString& model_name);
    Q_INVOKABLE QString Request4ModelBlueprint(const QString& model_name);
    Q_INVOKABLE bool Registry4Model(const QString& model_name, const QString& str_model_data);
    Q_INVOKABLE bool DeleteModel(const QString& model_name);
    Q_INVOKABLE QString GetBackendStatus();
    Q_INVOKABLE QStringList Request4ModelNameList();

    /// <summary>
    /// Transfer a coroutine handle to the pixel space, using the generator-defined
    /// GetValue/Resume protocol.
    /// </summary>
    ModelGenerator<SingleAutomata> OfferDynamicModel(const QString& model_name);

signals:
    QStringList update4NameList();
    void operationSucceeded(const QString& message);
    void operationFailed(const QString& message);

private:
    bool LoadGraphModel(const QString& model_name, GraphModel& graph_model, QString& error);
    QString RenderModelLayers(const GraphModel& graph_model);
    QString BuildPreview(const GraphModel& graph_model);
    static int KindIndex(const std::string& func_name);
    static QString KindName(int kind);
    static bool IsStaticAlgorithm(const std::string& func_name);
    static bool IsDynamicAlgorithm(const std::string& func_name);
    static std::string EnvString(const char* name, const char* fallback);
    static int EnvInt(const char* name, int fallback);

    std::shared_ptr<XMLStreamConvertToModelDataStrategy> sp_stream_to_modeldata;
    std::shared_ptr<IModelDataConvertToStreamStrategy> sp_modeldata_to_stream;
    std::shared_ptr<RenderCU> sp_render_cu;
    std::shared_ptr<FileManager> sp_file_manager;
    RedisClient redis_client;
    DatabaseManager database_manager;
    std::mutex storage_mutex;
};

#endif // !RENDER_KERNEL