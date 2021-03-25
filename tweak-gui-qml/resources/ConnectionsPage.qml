/**
 * @file ConnectionsPage.qml
 * @ingroup GUI
 *
 * @brief Page where the user can add and delete connections.
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
            name: "dummy"
            uri: "dummy"
            contextType: "nng"
            autoConnect: false
            connectionId: -1
        }
    }

    Settings {
        id: settings
        category: "Connections"

        property string list: '[{"name":"default","uri":"tcp://0.0.0.0:7777","contextType":"nng"}]'
    }

    Component.onCompleted: {
        if (settings.list)
        {
            var model = JSON.parse(settings.list);

            connectionModel.clear();
            for (var i = 0; i < model.length; i++)
            {
                var item = model[i];

                if (item.autoConnect)
                {
                    item.connectionId = tweak.addClient(item.name,
                                                        item.contextType,
                                                        "role=client",
                                                        item.uri);
                }
                else
                {
                    item.connectionId = -1;
                }

                connectionModel.append(item);
            }
        }
    }

    function saveModel() {
        var model = [];
        for (var i = 0; i < connectionModel.count; i++)
        {
            var item = connectionModel.get(i);
            model.push(item);
        }
        settings.list = JSON.stringify(model);
    }


    ToolBar {
        Layout.fillWidth: true

        RowLayout {
            Layout.fillWidth: true

            TweakToolButton {
                text: "Add"
                iconSource: "qrc:/images/button-plus.png"

                onClicked: {
                    var item = {connectionId: -1,  contextType: "nng"};
                    connectionModel.append(item);
                }
            }
            ToolSeparator {
                Layout.fillHeight: true
            }
            TweakToolButton {
                text: "Clear"
                iconSource: "qrc:/images/button-minus.png"

                onClicked: {
                    connectionModel.clear();
                    saveModel();
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

                    TextField {
                        text: name
                        Layout.fillWidth: true

                        onTextChanged: {
                            name  = text;
                            saveModel();
                        }
                    }

                    Label {
                        text: "Connect automatically"
                    }

                    CheckBox {
                        checked: autoConnect
                        onCheckedChanged: {
                            autoConnect = checked;
                            saveModel();
                        }
                    }

                    Label {
                        text: "URI"
                        enabled: connectionId < 0
                    }

                    TextField {
                        text: uri
                        Layout.fillWidth: true

                        enabled: connectionId < 0

                        onTextChanged: {
                            uri = text;
                            saveModel();
                        }
                    }

                    Label {
                        text: "Context type"
                        enabled: connectionId < 0
                    }

                    ComboBox {
                        property string cType: contextType

                        enabled: connectionId < 0

                        model: ["nng"]

                        onCTypeChanged: {
                            currentIndex = Math.max(find(cType), 0)
                        }

                        onActivated: {
                            cType = textAt(index);
                            contextType = cType;
                            saveModel();
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

                            text: "Connect"
                            enabled: connectionId < 0

                            onClicked: {
                                connectionId = tweak.addClient(name,
                                                               contextType,
                                                               "role=client",
                                                               uri);
                            }
                        }

                        Button {
                            id: disconnectButton

                            text: "Disconnect"
                            enabled: connectionId >= 0

                            onClicked: {
                                tweak.removeClient(connectionId);
                                connectionId = -1;
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                    Layout.fillWidth: true

                    spacing: 10

                    RoundButton {
                        text: "x"
                        onClicked: {
                            connectionModel.remove(index);
                            connectionId = -1;
                            saveModel();
                        }
                    }

                    Text {
                        text: connectionId < 0 ? "Disconnected" : "Connected"
                        color: connectionId < 0 ? "gray" : "green"
                    }
                }
            }
        }
    }
}
