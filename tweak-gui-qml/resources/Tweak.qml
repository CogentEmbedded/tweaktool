/**
 * @file Tweak.qml
 * @ingroup GUI
 *
 * @brief Template for tweak editor control.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
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
