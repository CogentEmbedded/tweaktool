/**
 * @file TweakScalar.qml
 * @ingroup GUI
 *
 * @brief Slider-based tweak editor control.
 *
 * @copyright 2020-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Universal 2.2

import TweakApplication 1.0

Tweak {

    columns: 4

    Slider {
        id: tweakSlider
        Layout.fillHeight: true
        Layout.fillWidth: true

        value: tweakValue

        width: 50

        onMoved: {
            tweakValue = value;
        }

        from: Number(meta.min)
        to: Number(meta.max)

        stepSize: Number(meta.step)
        snapMode: Slider.SnapAlways
    }

    SpinBox {
        id: spin

        Layout.minimumWidth: mainSpace.editorWidth + 80
        Layout.maximumWidth: mainSpace.editorWidth + 80

        editable: true

        property int decimals: meta.decimals
        property real scaler: Math.pow(10, decimals)
        value: Number(tweakValue) * scaler

        /*.. see https://doc.qt.io/qt-5/qml-int.html for details */
        from: Math.max(-2000000000, meta.min * scaler)
        to: Math.min(2000000000, meta.max * scaler)
        stepSize: Math.min(1, meta.step * scaler)

        validator: DoubleValidator {
            bottom: meta.min * spin.scaler
            top:  meta.max   * spin.scaler
        }

        onValueModified: {
            tweakValue = value / scaler;
        }

        textFromValue: function(value, locale) {
            return Number(value / scaler).toLocaleString(locale, 'f', decimals)
        }

        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text) * scaler
        }

        Component.onCompleted: {
            contentItem.selectByMouse = true;
        }
    }
}
