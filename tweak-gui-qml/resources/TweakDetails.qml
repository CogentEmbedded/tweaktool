/**
 * @file TweakDetails.qml
 * @ingroup GUI
 *
 * @brief Detailed view of tweakl.
 *
 * @copyright 2020-2023 Cogent Embedded Inc. ALL RIGHTS RESERVED.
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

GridLayout {
    rows: 2
    columns: 2

    Label {
        Layout.column: 0
        Layout.row: 0
        text: qsTr("URI:")
    }

    TextField {
        Layout.column: 1
        Layout.row: 0
        Layout.fillWidth: true

        text: uri
        readOnly: true

        selectByMouse: true
    }

    Label {
        Layout.column: 0
        Layout.row: 1
        text: qsTr("Raw value:")
    }

    TextField {
        Layout.column: 1
        Layout.row: 1
        Layout.fillWidth: true

        text: tweakValue
        selectByMouse: true

        onAccepted: {
            tweakValue = text
        }
    }

    Label {
        Layout.column: 0
        Layout.row: 2
        text: qsTr("Meta:")
    }

    TextField {
        Layout.column: 1
        Layout.row: 2
        Layout.fillWidth: true
        Layout.fillHeight: true

        text: meta.json
        readOnly: true
        selectByMouse: true
        verticalAlignment: TextInput.AlignTop

        placeholderText: qsTr("<none>")
    }
}
