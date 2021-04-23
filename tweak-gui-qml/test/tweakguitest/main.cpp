/**
 * @file main.cpp
 * @ingroup GUI
 *
 * @brief Main program for QML GUI tests.
 *
 * @copyright 2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#include "TweakComponents.hpp"
#include "TweakQmlApp.hpp"

#include <QQmlContext>
#include <QQmlEngine>
#include <QRandomGenerator>
#include <QtQuickTest>

class Fixture : public QObject
{
    Q_OBJECT

  public:
    Fixture()
    {
        /*.. Initialize resources from the static library */
        Q_INIT_RESOURCE(main);

        tweak2::registerQmlTypes();
    }

  public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        /*.. Make sure QSettings do not contain any stale values */
        QString rand = QString::number(QRandomGenerator::system()->generate());
        QSettings().clear();

        QApplication::setApplicationName("qt-test-" + rand);
        QApplication::setOrganizationName("qt-test-" + rand);
        QApplication::setOrganizationDomain("qt-test-" + rand);
        QApplication::setApplicationVersion("v2.0.0");

        engine->rootContext()->setContextProperty(
            "tweak", new tweak2::TweakApplication());
    }
};

QUICK_TEST_MAIN_WITH_SETUP(tweakgui, Fixture)

#include "main.moc"
