/**
 * @file AutoSelectControl.qml
 * @ingroup GUI
 *
 * @brief Automatic selection of control editor.
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

import TweakApplication 1.0

ColumnLayout {
    property bool isReadonlyControl: meta.readonly
    property bool isToggleControl: meta.toggle

    TweakScalar {
        id: scalarEditor

        visible: !isReadonlyControl && !isToggleControl
    }

    TweakLabel {
        id: labelEditor

        visible: isReadonlyControl && !isToggleControl
    }

    TweakToggle {
        id: toggleEditor

        visible: isToggleControl

        isEnabled: !isReadonlyControl
    }
}
