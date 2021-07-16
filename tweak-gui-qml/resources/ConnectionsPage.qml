/**
 * @file ConnectionsPage.qml
 * @ingroup GUI
 *
 * @brief Page where the user can add and delete connections.
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

import Qt.labs.settings 1.0

import SortFilterProxyModel 0.2

import TweakApplication 1.0

ColumnLayout {
    id: connectionsList

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.margins: 40

    ListModel {
        id: connectionModel

        ListElement {
            name: "default"
            uriSchema: "tcp"
            uriHost: "0.0.0.0"
            uriPort: 7777
            contextType: "nng"
            autoConnect: false
            connectionId: -1
        }
    }

    property var defaultItem: {
        "name": "default",
        "autoConnect": false,
        "uriSchema": "tcp",
        "uriHost": "127.0.0.1",
        "uriPort": 7777,
        "contextType": "nng",
        "connectionId": -1
    }

    Settings {
        id: settings
        category: "Connections"

        property string list: JSON.stringify([defaultItem])
    }

    function combineUri(schema, host, port) {
        return (schema !== "" ? schema : "?") + "://"
                + (host !== "" ? host : "?") + ":" + port + "/"
    }

    Component.onCompleted: {
        if (settings.list) {
            var model = JSON.parse(settings.list)

            connectionModel.clear()
            for (var i = 0; i < model.length; i++) {
                var item = model[i]

                if (item.autoConnect) {
                    var uri = combineUri(item.uriSchema, item.uriHost,
                                         item.uriPort)
                    item.connectionId = tweak.addClient(item.name,
                                                        item.contextType,
                                                        "role=client", uri)
                } else {
                    item.connectionId = -1
                }

                connectionModel.append(item)
            }
        }
    }

    function saveModel() {
        var model = []
        for (var i = 0; i < connectionModel.count; i++) {
            var item = connectionModel.get(i)
            model.push(item)
        }
        settings.list = JSON.stringify(model)
    }


    ToolBar {
        Layout.fillWidth: true

        RowLayout {
            Layout.fillWidth: true

            TweakToolButton {
                id: addConnectionButton
                objectName: "addConnectionButton"
                text: "Add"
                iconSource: "qrc:/images/button-plus.png"

                onClicked: {
                    var item = defaultItem
                    connectionModel.append(item)
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            TweakToolButton {
                id: clearConnectionsButton
                objectName: "clearConnectionsButton"

                text: "Clear"
                iconSource: "qrc:/images/button-minus.png"

                enabled: connectionModel.count > 0

                onClicked: {
                    connectionModel.clear()
                    saveModel()
                }
            }
        }
    }

    Text {
        text: "Please click [Add] button to add connections"

        horizontalAlignment: Qt.AlignCenter

        Layout.fillHeight: true
        Layout.fillWidth: true

        visible: connectionModel.count <= 0
    }

    ListView {
        model: connectionModel

        Layout.fillHeight: true
        Layout.fillWidth: true

        visible: connectionModel.count > 0
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            snapMode: "SnapAlways"
        }

        spacing: 5

        delegate: GroupBox {

            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            anchors.margins: 40

            RowLayout {
                anchors.left: parent.left
                anchors.right: parent.right

                GridLayout {
                    Layout.fillHeight: true
                    columns: 2

                    Label {
                        text: "Name"
                    }

                    ValidatedTextField {
                        id: nameField
                        objectName: "nameField"

                        enabled: connectionId < 0

                        text: name
                        onTextChanged: {
                            name = text
                            saveModel()
                        }

                        textInvalidMessage: "Name must not be empty"
                    }

                    Label {
                        text: "Connect automatically"
                    }

                    CheckBox {
                        checked: autoConnect
                        onCheckedChanged: {
                            autoConnect = checked
                            saveModel()
                        }
                    }

                    Label {
                        text: "URI scheme"
                        enabled: connectionId < 0
                    }

                    ComboBox {
                        id: uriSchemaSelector
                        objectName: "uriSchemaSelector"
                        enabled: connectionId < 0

                        model: ["tcp"]

                        currentIndex: Math.max(find(uriSchema), 0)
                        Component.onCompleted: {
                            currentIndex = Math.max(find(uriSchema), 0)
                        }

                        onActivated: {
                            uriSchema = textAt(index)
                            saveModel()
                        }
                    }

                    Label {
                        text: "URI host"
                        enabled: connectionId < 0
                    }

                    ValidatedTextField {
                        id: uriHostField
                        objectName: "uriHostField"
                        enabled: connectionId < 0

                        text: uriHost

                        /*.. no whitespaces are allowed and the name shall start with a letter or a digit */
                        textValidator: /^[a-zA-Z0-9]\S*/
                        textInvalidMessage: "Enter valid host name or IP address"

                        onTextChanged: {
                            uriHost = text
                            saveModel()
                        }
                    }

                    Label {
                        text: "URI port"
                        enabled: connectionId < 0
                    }

                    SpinBox {
                        id: uriPortSelector
                        enabled: connectionId < 0
                        editable: true

                        from: 1
                        to: 65535

                        value: uriPort

                        onValueModified: {
                            uriPort = value
                            saveModel()
                        }
                    }

                    Label {
                        text: "URI"
                        enabled: connectionId < 0
                    }

                    Label {
                        id: uriLabel
                        objectName: "uriLabel"

                        text: combineUri(uriSchema, uriHostField.text,
                                         uriPortSelector.value)
                        enabled: connectionId < 0
                    }

                    Label {
                        text: "Context type"
                        enabled: connectionId < 0
                    }

                    ComboBox {
                        enabled: connectionId < 0

                        model: ["nng"]

                        currentIndex: Math.max(find(contextType), 0)
                        Component.onCompleted: {
                            currentIndex = Math.max(find(contextType), 0)
                        }

                        onActivated: {
                            contextType = textAt(index)
                            saveModel()
                        }
                    }

                    Label {
                        text: "Role"
                        enabled: connectionId < 0
                    }

                    TextField {
                        text: "client"
                        readOnly: true
                        enabled: connectionId < 0
                    }

                    RowLayout {
                        Button {
                            id: connectButton
                            objectName: "connectButton"

                            text: "Connect"
                            enabled: connectionId < 0 && nameField.textIsValid
                                     && uriHostField.textIsValid

                            onClicked: {
                                var uri = combineUri(uriSchema,
                                                     uriHost, uriPort)
                                connectionId = tweak.addClient(name,
                                                               contextType,
                                                               "role=client",
                                                               uri)
                            }
                        }

                        Button {
                            id: disconnectButton

                            text: "Disconnect"
                            enabled: connectionId >= 0

                            onClicked: {
                                tweak.removeClient(connectionId)
                                connectionId = -1
                            }
                        }

                        Button {
                            id: deleteButton
                            text: "Delete"
                            onClicked: {

                                if (connectionId >= 0) {
                                    tweak.removeClient(connectionId)
                                    connectionId = -1
                                }

                                connectionModel.remove(index)
                                saveModel()
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                    Layout.fillWidth: true

                    spacing: 10

                    Text {
                        text: connectionId < 0 ? "Disconnected" : "Connected"
                        color: connectionId < 0 ? "gray" : "green"
                    }
                }
            }
        }
    }
}
