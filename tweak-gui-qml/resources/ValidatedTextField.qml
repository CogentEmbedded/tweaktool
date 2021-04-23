/**
 * @file ValidatedTextField.qml
 * @ingroup GUI
 *
 * @brief Text field with validation.
 *
 * @copyright 2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
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
