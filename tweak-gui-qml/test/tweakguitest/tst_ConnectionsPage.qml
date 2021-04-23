
/**
 * @file tst_ConnectionsPage.qml
 * @ingroup GUI
 *
 * @brief Unit-test for connections page.
 *
 * @copyright 2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */
import QtQuick 2.0
import QtTest 1.0

/*.. importing custom controls that reside in the root of resources */
import "qrc:/."

TestCase {
    id: testCase
    name: "ConnectionsPageTest"
    when: windowShown
    width: 1024
    height: 768

    ConnectionsPage {
        id: page
    }

    function initTestCase() {}

    function cleanupTestCase() {}

    function test_01_clear_add() {
        var addConnectionButton = findChild(page, "addConnectionButton")
        var clearConnectionsButton = findChild(page, "clearConnectionsButton")

        verify(clearConnectionsButton.enabled)
        clearConnectionsButton.clicked()
        verify(!clearConnectionsButton.enabled)

        addConnectionButton.clicked()
        verify(clearConnectionsButton.enabled)
    }

    function test_02_TWEAK_135() {
        /*.. test for JIRA TWEAK-135 issue */
        var addConnectionButton = findChild(page, "addConnectionButton")
        var clearConnectionsButton = findChild(page, "clearConnectionsButton")

        clearConnectionsButton.clicked()
        addConnectionButton.clicked()
        verify(clearConnectionsButton.enabled)

        var uriSchemaSelector = findChild(page, "uriSchemaSelector")
        var uriHostField = findChild(page, "uriHostField")
        var connectButton = findChild(page, "connectButton")

        /*.. check defaults */
        compare(uriSchemaSelector.textAt(uriSchemaSelector.index), "tcp")
        compare(uriHostField.text, "127.0.0.1")

        /*.. check hostname validation */
        verify(connectButton.enabled)

        uriHostField.text = ""
        verify(!uriHostField.textIsValid)
        verify(!connectButton.enabled)

        uriHostField.text = "some"
        verify(uriHostField.textIsValid)

        uriHostField.text = "0.0.0.0"
        verify(uriHostField.textIsValid)

        uriHostField.text = ""
        verify(!uriHostField.textIsValid)

        uriHostField.text = "$3"
        verify(!uriHostField.textIsValid)
    }
}
