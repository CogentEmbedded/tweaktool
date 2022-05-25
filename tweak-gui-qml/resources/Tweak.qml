/**
 * @file Tweak.qml
 * @ingroup GUI
 *
 * @brief Template for tweak editor control.
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
 * THE SOFTWARE.obtaining a copy
 */

import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Universal 2.2

import TweakApplication 1.0

GridLayout {
    columns: 2

    property bool nameVisible: true
    property string name: uri
    property string filter: mainSpace.cutUrl

    columnSpacing: 10

    Button {
        id: favoritesAdd
        property int imageSize: 24

        visible: !isFavorite

        Layout.minimumHeight: imageSize
        Layout.maximumHeight: imageSize
        Layout.minimumWidth: imageSize
        Layout.maximumWidth: imageSize

        contentItem: Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "qrc:/images/button-favorites-add.png"
        }

        onPressed: {
            tweak.addFavorite(name)
        }
    }

    Button {
        id: favoritesRemove
        property int imageSize: 24

        visible: isFavorite

        Layout.minimumHeight: imageSize
        Layout.maximumHeight: imageSize
        Layout.minimumWidth: imageSize
        Layout.maximumWidth: imageSize

        contentItem: Image {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "qrc:/images/button-favorites-remove.png"
        }

        onPressed: {
            tweak.removeFavorite(name)
        }
    }

    Text {
        id: nameText

        visible: nameVisible

        Layout.row: 0
        Layout.column: 1
        Layout.minimumWidth: 200

        text: name.replace(RegExp(filter), "")

        ToolTip {
            text: description
            visible: description ? ma.containsMouse : false
            delay: 1000
        }

        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
        }
    }
}
