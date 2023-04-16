/**
 * @file AutoSelectControl.qml
 * @ingroup GUI
 *
 * @brief Automatic selection of control editor.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

Item {
    property bool isReadonlyControl: meta.readonly
    property var controlType: meta.controlType
    property var options: meta.options
    property var numericEditor: (controlType == TweakMetadata.Spinbox
                                 || controlType == TweakMetadata.Slider)

    TweakEditorScalar {
        id: scalarEditor

        visible: numericEditor && !isReadonlyControl
    }

    TweakEditorLabel {
        id: labelEditor
        anchors.fill: parent

        visible: numericEditor && isReadonlyControl
    }

    TweakEditorBox {
        id: boxEditor

        visible: controlType == TweakMetadata.Editbox
        isEnabled: !isReadonlyControl
    }

    TweakEditorToggle {
        id: toggleEditor

        visible: controlType == TweakMetadata.Checkbox
        isEnabled: !isReadonlyControl
    }

    TweakEditorEnum {
        id: enumEditor

        visible: controlType == TweakMetadata.Combobox
        isEnabled: !isReadonlyControl
    }

    TweakEditorButton {
        id: button

        visible: controlType == TweakMetadata.Button
    }
}
