/**
 * @file TweakComponents.cpp
 * @ingroup GUI
 *
 * @brief Registration logic for all QML components.
 *
 * @copyright 2021-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

