/**
 * @file TweakEditorBase.qml
 * @ingroup GUI
 *
 * @brief Template for tweak editor control.
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


import TweakApplication 1.0

GridLayout {
    /*.. width of all property editors, should fit large numbers */
    property int defaultEditorWidth: 120

    /*.. we always fill the parent rectangle */
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right

    property int commonColumns: 3
    columns: commonColumns
    rows: 1

    property bool nameVisible: true
    property string name: uri

    columnSpacing: 10
    rowSpacing: 10

    ItemButton {
        id: favoritesAdd

        imageSource: "qrc:/images/button-favorites-add.png"
        visible: !isFavorite
        toolTip: qsTr("Add to favorites")

        onClicked: {
            tweak.addFavorite(name)
        }
    }

    ItemButton {
        id: favoritesRemove

        imageSource: "qrc:/images/button-favorites-remove.png"
        visible: isFavorite
        toolTip: qsTr("Remove from favorites")

        onClicked: {
            tweak.removeFavorite(name)
        }
    }

    ItemButton {
        id: detailsButton

        imageSource: "qrc:/images/button-details.png"

        onClicked: {
            if (tweakRectangle.state == "expanded") {
                tweakRectangle.state = ""
            } else if (tweakList.interactive) {
                tweakRectangle.state = "expanded"
            }
        }

        enabled: tweakList.interactive || tweakRectangle.state == "expanded"
        flat: !(tweakList.interactive || tweakRectangle.state == "expanded")
        highlighted: tweakRectangle.state == "expanded"
    }

    Text {
        id: nameText

        visible: nameVisible

        Layout.row: 0
        Layout.column: 2
        Layout.minimumWidth: Math.max(150, tweakRectangle.width * 0.15)
        Layout.maximumWidth: tweakRectangle.width * 0.3

        text: name.replace(tweakList.filter, "")
        elide: Text.ElideMiddle

        ToolTip {
            /*.. TODO: display description */
            text: name
            visible: name ? ma.containsMouse : false
            delay: 1000
        }

        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
        }
    }
}
