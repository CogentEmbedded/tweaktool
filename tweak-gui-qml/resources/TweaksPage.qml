/**
 * @file TweaksPage.qml
 * @ingroup GUI
 *
 * @brief Main page with tweak controls.
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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Universal 2.2
import Qt.labs.settings 1.0

import SortFilterProxyModel 0.2

import TweakApplication 1.0

SplitView {
    id: mainSpace

    /*.. width of all property editors, should fit large numbers */
    property int editorWidth: 120

    Layout.fillWidth: true
    Layout.fillHeight: true

    orientation: Qt.Horizontal
    resizing: true

    property string listFilter: "."
    property string favoritesRegEx: tweak.favoritesRegEx
    property string cutUrl: ""
    property bool favoritesAreSelected: false

    Settings {
        id: settings
        category: "Display"

        /// Show children tweaks in parent nodes, off by default
        property bool showChildrenInParent: false
    }

    function updateFilter() {
        var index = tweakTree.currentIndex
        var selectionFilter = tweak.tree.selectionToRegExp(index)

        if (selectionFilter === "^/Favorites/") {
            listFilter = favoritesRegEx
            cutUrl = ""
            favoritesAreSelected = true
        } else {
            listFilter = selectionFilter
            if (!settings.showChildrenInParent)
                listFilter += "[^/]*$"
            cutUrl = selectionFilter
            favoritesAreSelected = false
        }
    }

    onFavoritesRegExChanged: updateFilter()

    ColumnLayout {
        Layout.fillHeight: true
        Layout.minimumWidth: 150

        TreeView {
            id: tweakTree

            model: tweak.tree

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 150

            headerVisible: false

            onCurrentIndexChanged: updateFilter()
            selectionMode: SelectionMode.SingleSelection

            TableViewColumn {
                title: "Component Tree"
                role: "item"

                width: tweakTree.width

                delegate: Row {
                    spacing: 5
                    Image {
                        function selectIcon(itemType) {
                            if (itemType == TweakTreeModel.Favorites) {
                                return "qrc:/images/tree-favorites.png"
                            } else if (itemType == TweakTreeModel.Connection) {
                                return "qrc:/images/tree-connection.png"
                            }

                            /*.. normal leaves and root have no icon */
                            return ""
                        }

                        source: selectIcon(styleData.value.itemType)

                        visible: source != ""

                        fillMode: Image.PreserveAspectFit

                        anchors {
                            verticalCenter: parent.verticalCenter
                            rightMargin: 10
                        }
                        height: 16
                        width: 16
                    }

                    Text {
                        anchors {
                            leftMargin: 10
                            verticalCenter: parent.verticalCenter
                        }

                        horizontalAlignment: Qt.AlignLeft
                        color: styleData.textColor
                        elide: styleData.elideMode
                        text: styleData.value ? styleData.value.name : null
                    }
                }
            }
        }

        Image {
            Layout.fillWidth: true
            Layout.minimumHeight: 50
            Layout.minimumWidth: 150

            fillMode: Image.PreserveAspectFit

            id: cogentLogo
            source: "qrc:/images/cogent-logo.png"
        }
    }

    SortFilterProxyModel {
        id: tweakProxyModel

        sourceModel: tweak

        delayed: true
        dynamicSortFilter: false

        filters: RegExpFilter {
            roleName: "uri"
            pattern: listFilter
        }
    }

    ColumnLayout {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.minimumWidth: 450

        ToolBar {
            Layout.fillWidth: true
            Layout.minimumHeight: 32
            z: 1

            RowLayout {
                Layout.fillWidth: true

                TweakToolButton {
                    text: "List"
                    iconSource: "qrc:/images/button-grid.png"

                    onClicked: {
                        tweakList.columns = 1
                    }

                    checkable: true
                    checked: tweakList.columns == 1
                }
                ToolSeparator {}
                TweakToolButton {
                    text: "Grid"
                    iconSource: "qrc:/images/button-list.png"

                    onClicked: {
                        tweakList.columns = 5
                    }
                    checkable: true
                    checked: tweakList.columns > 1
                }
                ToolSeparator {
                    visible: !favoritesAreSelected
                }
                TweakToolButton {
                    text: "Show children"
                    visible: !favoritesAreSelected
                    checkable: true
                    checked: settings.showChildrenInParent
                    iconSource: checked ? "qrc:/images/button-tree-on.png" : "qrc:/images/button-tree-off.png"
                    onClicked: {
                        settings.showChildrenInParent = checked
                        mainSpace.updateFilter()
                    }
                }
                ToolSeparator {
                    visible: favoritesAreSelected
                }
                TweakToolButton {
                    visible: favoritesAreSelected

                    text: "Clear All Favorites"
                    iconSource: "qrc:/images/button-clear-all.png"

                    onClicked: {
                        tweak.clearFavorites()
                    }
                }
            }
        }

        GridView {
            id: tweakList
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 450

            property int scrollBarThickness: 15

            boundsBehavior: Flickable.StopAtBounds

            property int columns: 1
            property real minWidth: 700
            property int workAreaWidth: width - tweakList.scrollBarThickness

            cellWidth: workAreaWidth / Math.min(
                           columns, Math.floor(workAreaWidth / minWidth) + 1)
            cellHeight: 50

            ScrollBar.vertical: TweakScrollBar {
                barThickness: tweakList.scrollBarThickness
                snapMode: "SnapAlways"
            }

            model: tweakProxyModel

            delegate: AutoSelectControl {
                anchors.margins: tweakList.columns > 0 ? 40 : 0
                width: tweakList.cellWidth
                height: tweakList.cellHeight
            }

            visible: tweakProxyModel.count > 0
        }

        Text {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 450

            text: favoritesAreSelected ? "No favorites are visible." : "No items match selection. Please make different selction to view items."
            visible: tweakProxyModel.count <= 0
        }
    }
}
