 /**
 * @file AutoSelectControl.qml
 * @ingroup GUI
 *
 * @brief Automatic selection of control editor.
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

ColumnLayout {
    property bool isReadonlyControl: meta.readonly
    property var controlType: meta.controlType
    property var options: meta.options
    property var numericEditor: (meta.controlType == TweakMetadata.Spinbox || meta.controlType == TweakMetadata.Slider) 

    TweakScalar {
        id: scalarEditor

        visible: numericEditor && !isReadonlyControl 
    }

    TweakLabel {
        id: labelEditor

        visible: numericEditor && isReadonlyControl 
    }

    TweakToggle {
        id: toggleEditor

        visible: controlType == TweakMetadata.Checkbox

        isEnabled: !isReadonlyControl
    }

    TweakEnum {
        id: enumEditor

        visible: controlType == TweakMetadata.Combobox

        isEnabled: !isReadonlyControl
    }

    TweakButton {
        id: button

        visible: controlType == TweakMetadata.Button
    }
}
