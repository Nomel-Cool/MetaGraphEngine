#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <filesystem>
#include <iostream>

#include "RenderKernel.h"
#include "PixelSpace.h"

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<GraphFactory>("CustomModules", 1, 0, "QGraphFactory");
    qmlRegisterType<GraphStudio>("CustomModules", 1, 0, "QGraphStudio");

    engine.load(QUrl(QStringLiteral(u"qrc:resources/qmls/MainPage.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}