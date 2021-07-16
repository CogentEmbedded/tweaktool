/**
 * @file TweakComponents.cpp
 * @ingroup GUI
 *
 * @brief Registration logic for all QML components.
 *
 * @copyright 2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */


#include "TweakTreeModel.hpp"
#include "TweakQmlApp.hpp"

#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQuickView>

namespace qqsfpm
{
extern void registerQQmlSortFilterProxyModelTypes();
extern void registerFiltersTypes();
extern void registerProxyRoleTypes();
extern void registerSorterTypes();
} // namespace qqsfpm


namespace tweak2
{

void registerQmlTypes()
{
    qqsfpm::registerQQmlSortFilterProxyModelTypes();
    qqsfpm::registerFiltersTypes();
    qqsfpm::registerProxyRoleTypes();
    qqsfpm::registerSorterTypes();

    qmlRegisterType<TweakApplication>("TweakApplication", 1, 0, "TweakApplication");
    qmlRegisterType<TweakTreeModel>("TweakApplication", 1, 0, "TweakTreeModel");
    qmlRegisterType<TweakMetadata>("TweakApplication", 1, 0, "TweakMetadata");
    qmlRegisterType<OptionsModel>("TweakApplication", 1, 0, "OptionsModel");

    qRegisterMetaType<QImage>();
}

}

