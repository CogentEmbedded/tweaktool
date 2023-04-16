/**
 * @file Tweak.qml
 * @ingroup GUI
 *
 * @brief Template for tweak editor control.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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


import TweakApplication 1.0

Rectangle {
    id: tweakRectangle

    /*.. Layout */
    property bool displayDescription: description && description != "" ? mainSpace.displayDescription : false
    property bool displayDetails: false
    property int editorRows: 1 /*.. until matrices are supported */
    property int descriptionRows: displayDescription ? 1 : 0
    property int detailsRows: displayDetails ? 4 : 0

    /* .. Dimensions */
    property int rows: editorRows + descriptionRows + detailsRows
    property int rowHeight: ListView.view.cellHeight
    width: ListView.view.cellWidth
    height: rowHeight * rows

    /*.. Highlighting on mouse move */
    Rectangle {
        anchors.fill: parent

        visible: ma.containsMouse
        color: Qt.darker(parent.color, 1.05)
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        hoverEnabled: true

        ColumnLayout {
            anchors.fill: parent

            AutoSelectControl {
                id: tweakEditor

                Layout.fillWidth: true
                Layout.minimumHeight: rowHeight * editorRows
                Layout.maximumHeight: rowHeight * editorRows
                Layout.alignment: Qt.AlignTop
            }

            Text {
                id: descriptionText

                visible: displayDescription

                Layout.fillWidth: true
                Layout.minimumHeight: rowHeight * descriptionRows
                Layout.maximumHeight: rowHeight * descriptionRows
                Layout.topMargin: -5

                text: description

                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                verticalAlignment: Text.AlignTop
                minimumPointSize: 10
                fontSizeMode: Text.Fit

                color: "darkgray"
            }

            TweakDetails {
                id: detailsView

                Layout.fillWidth: true
                height: rowHeight * detailsRows

                visible: displayDetails
            }
        }
    }

    states: [
        State {
            name: "expanded"
            PropertyChanges {
                target: tweakRectangle
                displayDetails: true
            }
            PropertyChanges {
                target: tweakList
                interactive: false
            }
            //            PropertyChanges {
            //                target: tweakList
            //                restoreEntryValues: false
            //                // (tweakList.contentY + tweakList.contentHeight) - (tweakRectangle.y + tweakRectangle.height)
            ////                contentY: tweakList.contentY + tweakList.contentHeight < tweakRectangle.y + tweakRectangle.height ?
            ////                          tweakList.contentY + 250 : tweakList.contentY
            //                contentY: tweakRectangle.y
            //            }
            PropertyChanges {
                target: tweakList.ScrollBar.vertical
                visible: false
            }
        }
    ]
}
