/**
 * @file main.qml
 * @ingroup GUI
 *
 * @brief Main application window.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Universal 2.2

import TweakApplication 1.0

ApplicationWindow {
    id: window

    visible: true
    flags: Qt.WindowCloseButtonHint | Qt.WindowTitleHint | Qt.WindowMinMaxButtonsHint

    minimumWidth: 900
    minimumHeight: 750

    title: "Cogent Tweak Tool v2 Preview"

    header:     TabBar{
        Layout.fillWidth: true

        id: bar

        currentIndex: 0

        TabButton {
            text: "Connections"
        }

        TabButton {
            text: "Tweaks"
        }

        TabButton {
            text: "Scripts"
        }
    }

    StackLayout {

        anchors.fill: parent

        currentIndex: bar.currentIndex

        ConnectionsPage {
        }

        TweaksPage {
        }

        Item {

        }
    }

    footer: ToolBar {

        RowLayout {
            anchors.fill: parent
            Label { text: "Cogent Tweak Tool 2: Preview" }
        }
    }
}
