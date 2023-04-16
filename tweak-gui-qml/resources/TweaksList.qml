/**
 * @file TweaksList.qml
 * @ingroup GUI
 *
 * @brief List of tweak controls.
 *
 * @copyright 2020-2023 Cogent Embedded Inc. ALL RIGHTS RESERVED.
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
 * THE SOFTWARE.obtaining a copy
 */
import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

import Qt.labs.settings 1.0

import SortFilterProxyModel 0.2

import TweakApplication 1.0

ListView {
    id: tweakList

    /*.. this is cut from the beginning of the URI */
    property var filter: RegExp(mainSpace.cutUrl)

    /*.. Layout */
    spacing: 0

    /*.. Scrolling */
    clip: true
    property int scrollBarThickness: 15
    snapMode: ListView.SnapToRow
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: TweakScrollBar {
        barThickness: parent.scrollBarThickness
        snapMode: ScrollBar.SnapAlways
    }

    /*.. Grid layout */
    property real minCellWidth: 700
    property int workAreaWidth: width - scrollBarThickness - 5
    property int cellWidth: workAreaWidth
    property int cellHeight: 45

    /*.. Model */
    delegate: Tweak {
        color: userUriFilter.enabled ? "#eeffee" : "#ffffff"
    }
}
