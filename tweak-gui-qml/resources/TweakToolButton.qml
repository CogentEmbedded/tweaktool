/**
 * @file TweakToolButton.qml
 * @ingroup GUI
 *
 * @brief Custom button for the toolbar.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Universal 2.2

ToolButton {
    id: tweakTB

    property string iconSource

    Layout.fillHeight: true

    contentItem: RowLayout {
        Image {
            id: buttonImage
            source: tweakTB.iconSource

            visible: tweakTB.iconSource != ''

            Layout.fillHeight: true
            Layout.minimumWidth: 32
            Layout.maximumWidth: 32
            Layout.minimumHeight: 32
            Layout.maximumHeight: 32

            fillMode: Image.PreserveAspectFit
            antialiasing: true
        }

        Text {
            id: buttonText

            visible: tweakTB.text != ''

            Layout.fillHeight: true
            Layout.minimumHeight: font.pixelSize

            text: tweakTB.text
            font: tweakTB.font
            opacity: enabled ? 1.0 : 0.3
            color: tweakTB.down ? "#026FA5" : "#013E5A"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
