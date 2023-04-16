/**
 * @file TweakToolButton.qml
 * @ingroup GUI
 *
 * @brief Custom button for the toolbar.
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
import QtQuick.Controls.Universal 2.2

ToolButton {
    id: tweakTB

    property string iconSource

    Layout.fillHeight: true

    contentItem: RowLayout {
        Image {
            id: buttonImage
            source: tweakTB.iconSource

            visible: tweakTB.iconSource != ''

            Layout.fillHeight: true
            Layout.minimumWidth: 32
            Layout.maximumWidth: 32
            Layout.minimumHeight: 32
            Layout.maximumHeight: 32

            fillMode: Image.PreserveAspectFit
            antialiasing: true
        }

        Text {
            id: buttonText

            visible: tweakTB.text != ''

            Layout.fillHeight: true
            Layout.minimumHeight: font.pixelSize

            text: tweakTB.text
            font: tweakTB.font
            opacity: enabled ? 1.0 : 0.3
            color: tweakTB.down ? "#026FA5" : "#013E5A"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Rectangle {
          implicitWidth: 40
          implicitHeight: 40

          color: tweakTB.enabled && (tweakTB.highlighted || tweakTB.checked) ? tweakTB.Universal.accent : "transparent"

          Rectangle {
              width: parent.width
              height: parent.height
              visible: tweakTB.down || tweakTB.hovered
              color: tweakTB.down ? tweakTB.Universal.listMediumColor : tweakTB.Universal.listLowColor
          }
      }
}
