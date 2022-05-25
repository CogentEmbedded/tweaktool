/**
 * @file main.cpp
 * @ingroup GUI
 *
 * @brief Main QML GUI application.
 *
 * @copyright 2020-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
    app.setOrganizationName("Cogent Embedded, Inc.");
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
