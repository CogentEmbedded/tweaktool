/**
 * @file TweakNodeTree.test.cpp
 * @ingroup GUI
 *
 * @brief test suite for TweakQml component.
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
 * THE SOFTWARE.
 */

#include <QtTest/QtTest>

#include "TweakUriTree.hpp"
#include <stdexcept>

namespace tweak2
{


/**
 * @details Tests tree component. Spawns a tree, adds leafs and branches to it,
 * remove some leafs, check if everything works as expected.
 */
class TestTweakUriTree : public QObject
{
    Q_OBJECT

  private slots:
    void addTweaks()
    {
        TweakUriTree tree;

        tree.addTweak("/root/test", {0, 0});
        tree.addTweak("test", {0, 1});
        tree.addTweak("/1", {0, 2});
        tree.addTweak("2", {0, 3});
        tree.addTweak("/a/b//c/d/e/f/g/h/j", {0, 4});
        tree.addTweak("/root/test", {1, 0});
        tree.addTweak("test", {1, 1});
    }

    void addTweaksHard()
    {
        TweakUriTree tree;

        for (int i = 0; i < 16384; i++)
        {
            tree.addTweak(QString::asprintf("/q/%d/t/%d/z/%d/w/%d/h/%d", i % 17,
                                            i % 11, i % 5, i % 2, i),
                          {0, static_cast<tweak_id>(i)});
        }
    }

    void addNodeSignal()
    {
        TweakUriTree tree;
        QSignalSpy spyAdded(&tree, &TweakUriTree::newTweak);

        tree.addTweak("/root/test", {0, 0});
        QCOMPARE(spyAdded.count(), 1);
        tree.addTweak("test", {0, 1});
        QCOMPARE(spyAdded.count(), 2);
    }

    void addRemove()
    {
        TweakUriTree tree;

        tree.addTweak("/root/test", {0, 0});
        tree.removeTweak({0, 0});

        /*.. Adding the same tweak again is supported */
        tree.addTweak("/root/test", {0, 0});
        tree.removeTweak({0, 0});

        tree.addTweak("/root/test", {0, 0});
        tree.removeTweak({0, 0});

        tree.addTweak("/root/test", {0, 1});
        tree.removeTweak({0, 1});

        tree.addTweak("/root/test", {0, 2});
        tree.removeTweak({0, 2});
    }

    void addTweaksFailures()
    {
        TweakUriTree tree;
        QSignalSpy spyAdded(&tree, &TweakUriTree::newTweak);

        tree.addTweak("/root/test", {0, 0});

        /*.. Duplicate Tweak ID are not allowed. It does not matter if they are
         * with the same or with a different name */
        QVERIFY_EXCEPTION_THROWN(tree.addTweak("/root/test", {0, 0}),
                                 std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(tree.addTweak("/test1", {0, 0}),
                                 std::invalid_argument);

        QCOMPARE(spyAdded.count(), 1);
    }

    void listTweaks()
    {
        TweakUriTree tree;

        tree.addTweak("/root/test", {0, 0});
        tree.addTweak("/root/test", {0, 1});
        tree.addTweak("/root/test", {0, 3});

        QCOMPARE(tree.tweaks(QUrl("/root")).size(), 3);

        tree.removeAllTweaks();

        /*.. An attempt to list tweaks for non-existing path is not allowed.
         * This shall trigger an exception. */
        QVERIFY_EXCEPTION_THROWN(tree.tweaks(QUrl("/root")).size(),
                                 std::invalid_argument);
    }
};
} // namespace tweak2

#include "TweakNodeTree.test.moc"
QTEST_MAIN(tweak2::TestTweakUriTree)
