/**
 * @file TweakScrollBar.qml
 * @ingroup GUI
 *
 * @brief Custom wide ScrollBar for TweakTool.
 *
 * @copyright 2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */
import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Universal 2.2

ScrollBar {
    id: control

    property int barThickness: 15

    orientation: Qt.Vertical
    contentItem.implicitWidth: barThickness

    background: Rectangle {
        color: "#eeeeee"
    }
}
