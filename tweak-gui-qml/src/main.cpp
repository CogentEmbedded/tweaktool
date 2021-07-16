/**
 * @file main.cpp
 * @ingroup GUI
 *
 * @brief Main QML GUI application.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#include "TweakTreeModel.hpp"
#include "TweakQmlApp.hpp"
#include "TweakComponents.hpp"

#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQuickView>

using namespace tweak2;

tweak2::TweakApplication *initApplication(QGuiApplication &app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(PROJECT_SUMMARY);
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

    return new tweak2::TweakApplication();
}

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    /*.. Initialize resources from the static library */
    Q_INIT_RESOURCE(main);

    app.setApplicationName("Tweak Tool V2");
    app.setOrganizationName("Cogent Embedded Inc.");
    app.setOrganizationDomain("v2.tweaktool.cogentembedded.com");
    app.setApplicationVersion("v2.0.0");
    app.setQuitOnLastWindowClosed(true);
    app.setWindowIcon(QIcon("qrc:/images/tweak-icon.png"));

    tweak2::registerQmlTypes();

    tweak2::TweakApplication *tweakApp = initApplication(app);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("tweak", tweakApp);
    tweakApp = nullptr;

    engine.load("qrc:/main.qml");
    return app.exec();
}
