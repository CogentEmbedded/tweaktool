/**
 * @file TweakScalar.qml
 * @ingroup GUI
 *
 * @brief Slider-based tweak editor control.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
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

        stepSize: Math.pow(10, -meta.decimals)
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
