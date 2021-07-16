/**
 * @file TweakButton.qml
 * @ingroup GUI
 *
 * @brief Button control.
 *
 * @copyright 2020-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
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

    Item {
        Layout.fillWidth: true
    }

    Button {
        text: meta.caption
        Layout.minimumWidth: mainSpace.editorWidth + 80
        Layout.maximumWidth: mainSpace.editorWidth + 80
        onPressed: {
            tweakValue = true;
        }
        onReleased: {
            tweakValue = false;
        }
    }
}
