/**
 * @file TweakEnum.qml
 * @ingroup GUI
 *
 * @brief ComboBox control.
 *
 * @copyright 2020-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
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

    ComboBox {
        model: options
        currentIndex: model ? model.find(tweakValue) : -1
        Layout.minimumWidth: mainSpace.editorWidth + 80
        Layout.maximumWidth: mainSpace.editorWidth + 80
        textRole: "text"
        onActivated: {
            tweakValue = model.get(currentIndex)
        }
        enabled: isEnabled
    }
}
