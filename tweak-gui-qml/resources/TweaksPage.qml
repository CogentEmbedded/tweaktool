/**
 * @file TweaksPage.qml
 * @ingroup GUI
 *
 * @brief Main page with tweak controls.
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

    function updateFilter() {
        var index = tweakTree.currentIndex;
        var selectionFilter = tweak.tree.selectionToRegExp(index);

        if (selectionFilter === "^/Favorites/")
        {
            listFilter = favoritesRegEx;
            cutUrl = "";
            favoritesAreSelected = true;
        }
        else
        {
            listFilter = selectionFilter;
            cutUrl = selectionFilter;
            favoritesAreSelected = false;
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
                            if (itemType == TweakTreeModel.Favorites)
                            {
                                return "qrc:/images/tree-favorites.png";
                            }
                            else if (itemType == TweakTreeModel.Connection)
                            {
                                return "qrc:/images/tree-connection.png";
                            }

                            /*.. normal leaves and root have no icon */
                            return "";
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

            RowLayout {
                Layout.fillWidth: true

                TweakToolButton {
                    text: "List"
                    iconSource: "qrc:/images/button-grid.png"

                    onClicked: {
                        tweakList.columns = 1;
                    }

                    checkable: true
                    checked:  tweakList.columns == 1
                }
                ToolSeparator {

                }
                TweakToolButton {
                    text: "Grid"
                    iconSource: "qrc:/images/button-list.png"

                    onClicked: {
                        tweakList.columns = 5;
                    }
                    checkable: true
                    checked:  tweakList.columns > 1
                }
                ToolSeparator {
                    visible: favoritesAreSelected
                }
                TweakToolButton {
                    visible: favoritesAreSelected

                    text: "Clear All Favorites"
                    iconSource: "qrc:/images/button-clear-all.png"

                    onClicked: {
                        tweak.clearFavorites();
                    }
                }
            }
        }

        GridView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 450

            id: tweakList

            boundsBehavior: Flickable.StopAtBounds

            property int columns: 1
            property real minWidth: 700

            cellWidth: width / Math.min(columns, Math.floor(width / minWidth) + 1)
            cellHeight: 50

            ScrollBar.vertical: ScrollBar {
                snapMode: "SnapAlways"
            }

            model: tweakProxyModel

            delegate: AutoSelectControl {
                anchors.margins: tweakList.columns > 0 ? 40 : 0
                width: tweakList.cellWidth
                height: tweakList.cellHeight
            }
        }
    }
}
