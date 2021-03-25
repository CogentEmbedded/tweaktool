/**
 * @file TweakToggle.qml
 * @ingroup GUI
 *
 * @brief Boolean tweak editor control.
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

Tweak {
    columns: 4

    property bool isEnabled: false

    Item {
        Layout.fillWidth: true
    }

    Switch {
        checked: tweakValue

        onToggled: {
            tweakValue = checked ? 1 : 0;
        }

        enabled: isEnabled
    }
}
