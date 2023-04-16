/**
 * @file ValidatedTextField.qml
 * @ingroup GUI
 *
 * @brief Text field with validation.
 *
 * @copyright 2021-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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


ColumnLayout {
    id: dataField

    /* Data Bindings */
    property string text: ""

    /* Sizes */
    property real editorWidth: 0.7
    property real errorWidth: 0.3

    /* Enable/disable */
    property bool enabled: true

    /* Input Validation */
    property bool textIsValid: false
    property var textValidator: /.+/ /*.. Default is not-empty regex */
    property string textInvalidMessage: "{} is invalid"

    /* Customization properties */
    property color normalColor: "black"
    property color errorColor: "red"

    TextField {
        id: textInput

        Layout.fillWidth: true

        text: dataField.text

        selectByMouse: true
        enabled: dataField.enabled

        background: Rectangle {
            border.color: dataField.textIsValid ? dataField.normalColor : dataField.errorColor

            Behavior on border.color {
                enabled: dataField.textIsValid
                SequentialAnimation {
                    loops: 3
                    ColorAnimation {
                        from: dataField.normalColor
                        to: dataField.errorColor
                        duration: 300
                    }
                    ColorAnimation {
                        from: dataField.errorColor
                        to: dataField.normalColor
                        duration: 50
                    }
                }
            }
        }

        onTextChanged: {
            textIsValid = textValidator.test(text)

            if (textIsValid) {
                if (dataField.text != text) {
                    dataField.text = text
                }
            }
        }
    }

    Label {
        id: errorMessage

        Layout.fillWidth: true
        verticalAlignment: Qt.AlignVCenter

        enabled: dataField.enabled

        color: errorColor

        text: textInvalidMessage.replace('{}', textInput.text)
        visible: !dataField.textIsValid
    }
}
