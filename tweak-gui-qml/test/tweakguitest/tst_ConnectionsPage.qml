
/**
 * @file tst_ConnectionsPage.qml
 * @ingroup GUI
 *
 * @brief Unit-test for connections page.
 *
 * @copyright 2021-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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
