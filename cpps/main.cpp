#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QTcpSocket>

#include <cstdio>
#include <filesystem>

#include "RenderKernel.h"
#include "PixelSpace.h"

namespace {

bool CanConnect(const QString& host, quint16 port)
{
    QTcpSocket socket;
    socket.connectToHost(host, port);
    return socket.waitForConnected(700);
}

void LogHandler(QtMsgType, const QMessageLogContext&, const QString& message)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    QFile file(QCoreApplication::applicationDirPath() + "/logs/app.log");
    if (file.open(QIODevice::Append))
    {
        file.write(message.toUtf8());
        file.write("\n");
    }
}

void PrepareLogging()
{
    const QString log_dir = QCoreApplication::applicationDirPath() + "/logs";
    QDir().mkpath(log_dir);
    qInstallMessageHandler(LogHandler);
}
} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("QtGraphEngineKit"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Graph Engine Kit"));
    QGuiApplication::setOrganizationName(QStringLiteral("QtGraphEngineKit"));
    QGuiApplication::setApplicationVersion(QStringLiteral("1.0"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/resources/icons/app.png")));

    PrepareLogging();

    QQuickStyle::setStyle(QStringLiteral("Material"));

    const bool skip_preflight = qEnvironmentVariable("QGK_SKIP_PREFLIGHT") == QStringLiteral("1");
    const QString redis_host = qEnvironmentVariable("QGK_REDIS_HOST", QStringLiteral("127.0.0.1"));
    const quint16 redis_port = static_cast<quint16>(qEnvironmentVariableIntValue("QGK_REDIS_PORT"));
    const quint16 redis_port_final = redis_port == 0 ? 6379 : redis_port;

    QString startup_error;
    if (!skip_preflight)
    {
        if (!CanConnect(redis_host, redis_port_final))
            startup_error = QStringLiteral(
                "Cannot connect to Redis at %1:%2.\n\n"
                "Start Redis first, for example:\n"
                "  D:\\Redis\\Redis-x64-5.0.14.1\\redis-server.exe\n\n"
                "Verify with: redis-cli ping  (expect PONG)\n"
                "You can also set QGK_REDIS_HOST / QGK_REDIS_PORT.")
                .arg(redis_host).arg(redis_port_final);
        else if (!CanConnect(QStringLiteral("127.0.0.1"), 3306))
            startup_error = QStringLiteral(
                "Cannot connect to MySQL on 127.0.0.1:3306.\n\n"
                "Start the MySQL80 service, then create the database:\n"
                "  mysql -u root -p < sqls/schema.sql\n"
                "  mysql -u root -p < sqls/seed.sql\n\n"
                "Default database: meta_graph_model (user root / password 123456).\n"
                "You can override with QGK_DB_HOST / QGK_DB_USER / QGK_DB_PASSWORD / QGK_DB_NAME.");
    }

    QQmlApplicationEngine engine;
    if (!startup_error.isEmpty())
    {
        engine.rootContext()->setContextProperty(QStringLiteral("startupErrorText"), startup_error);
        engine.load(QUrl(QStringLiteral("qrc:/resources/qmls/StartupError.qml")));
        if (engine.rootObjects().isEmpty())
            return -1;
        return app.exec();
    }

    qmlRegisterType<GraphFactory>("CustomModules", 1, 0, "QGraphFactory");
    qmlRegisterType<GraphStudio>("CustomModules", 1, 0, "QGraphStudio");

    engine.load(QUrl(QStringLiteral("qrc:resources/qmls/MainPage.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}