/**
 * @file TweakQmlApp.test.cpp
 * @ingroup GUI
 *
 * @brief test suite for TweakQml component.
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
 * THE SOFTWARE.
 */

#include <QtTest/QtTest>
#include <QMutex>
#include <QWaitCondition>

#include "TweakQmlApp.hpp"
#include <tweak2/tweak2.h>
#include <tweak2/defaults.h>

#include <stdexcept>
#include <vector>
#include <random>
#include <errno.h>

namespace tweak2
{
enum { TEST_ITEM_COUNT = 1000 };

struct row_interval {
  int start;
  int end;
};

/**
 * @details Spawns a tweak server instance, starts TestTweakApplication instance,
 * add items, change items on both sides, remove items. See if TestTweakApplication
 * generate signals as expected.
 */
class TestTweakApplication : public QObject
{
  Q_OBJECT

  QMutex _lock;
  QWaitCondition _cond;
  bool _inserted;
  bool _updated;
  bool _serverUpdated;
  bool _removed;
  QVariant _value;
  float _serverValue;
  struct row_interval _row_interval;

  void rowsAboutToBeInserted(TweakApplication *tweakApplication,
    const QModelIndex &parent, int start, int end)
  {
    (void)tweakApplication;
    (void)parent;
    _lock.lock();
    struct row_interval tmp = { start, end };
    std::swap(_row_interval, tmp);
    _inserted = true;
    _cond.wakeAll();
    _lock.unlock();
  }

  void dataChanged(TweakApplication *tweakApplication,
    const QModelIndex &topLeft, const QModelIndex &bottomRight,
    const QVector<int> &roles)
  {
    (void)bottomRight;
    (void)roles;
    _lock.lock();
    _value = tweakApplication->data(topLeft, TweakApplication::ValueRole);
    _updated = true;
    _cond.wakeAll();
    _lock.unlock();
  }

  void rowsAboutToBeRemoved(TweakApplication *tweakApplication,
    const QModelIndex &parent, int start, int end)
  {
    (void)tweakApplication;
    (void)parent;
    _lock.lock();
    struct row_interval tmp = { start, end };
    std::swap(_row_interval, tmp);
    _removed = true;
    _cond.wakeAll();
    _lock.unlock();
  }

  QVariant waitChange() {
    QVariant result;
    _lock.lock();
    while (!_updated) {
        _cond.wait(&_lock);
    }
    result.swap(_value);
    _lock.unlock();
    return result;
  }

  struct row_interval waitInsert() {
    struct row_interval result = { -1, -1};
    _lock.lock();
    while (!_inserted) {
        _cond.wait(&_lock);
    }
    std::swap(_row_interval, result);
    _lock.unlock();
    return result;
  }
  struct row_interval waitRemove() {
    struct row_interval result = { -1, -1};
    _lock.lock();
    while (!_removed) {
      _cond.wait(&_lock);
    }
    std::swap(_row_interval, result);
    _lock.unlock();
    return result;
  }

  void itemChangeListener(tweak_id tweak_id) {
    _lock.lock();
    _serverValue = tweak_get_scalar_float(tweak_id);
    _serverUpdated = true;
    _cond.wakeAll();
    _lock.unlock();
  }

  static void itemChangeListenerAddapter(tweak_id tweak_id, void* cookie) {
    TestTweakApplication* testTweakApplication = static_cast<TestTweakApplication*>(cookie);
    testTweakApplication->itemChangeListener(tweak_id);
  }

  float waitServerUpdate() {
    float result;
    _lock.lock();
    while (!_serverUpdated) {
        _cond.wait(&_lock);
    }
    std::swap(_serverValue, result);
    _lock.unlock();
    return result;
  }

  private slots:
    void sanityTest() {
        tweak_initialize_library("nng", "role=server", TWEAK_DEFAULT_ENDPOINT);

        std::vector<float> ground_truth_values;
        std::vector<tweak_id> tweak_ids;
        ground_truth_values.resize(TEST_ITEM_COUNT);
        tweak_ids.resize(TEST_ITEM_COUNT);

        TweakApplication *tweakApplication = new TweakApplication(NULL, Qt::DirectConnection);
        tweak_set_item_change_listener(&TestTweakApplication::itemChangeListenerAddapter, this);

        QObject::connect(tweakApplication, &QAbstractItemModel::rowsAboutToBeInserted,
        [this, tweakApplication](const QModelIndex &parent, int start, int end) -> void {
            rowsAboutToBeInserted(tweakApplication, parent, start, end);
        });

        QObject::connect(tweakApplication, &QAbstractItemModel::dataChanged,
        [this, tweakApplication](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) -> void {
            dataChanged(tweakApplication, topLeft, bottomRight, roles);
        });

        QObject::connect(tweakApplication, &QAbstractItemModel::rowsAboutToBeRemoved,
        [this, tweakApplication](const QModelIndex &parent, int start, int end) -> void {
            rowsAboutToBeRemoved(tweakApplication, parent, start, end);
        });

        ConnectionId connectionId = tweakApplication->addClient("mock", "nng", "role=client", TWEAK_DEFAULT_ENDPOINT);
        for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
            char uri[100];
            snprintf(uri, sizeof(uri), "item_%d", item_no);
            ground_truth_values[item_no] = rand() * 2.f / (float) RAND_MAX - 1.f;
            _inserted = false;
            tweak_ids[item_no] = tweak_add_scalar_float(uri, uri, uri, ground_truth_values[item_no]);
            struct row_interval row_interval = waitInsert();
            QCOMPARE(row_interval.start, (int)item_no);
            QCOMPARE(row_interval.end, row_interval.start);
        }

        for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
            QString path = QString("item_%1").arg(item_no);
            QVariant value = tweakApplication->get(connectionId, path);
            float client_value = value.toFloat();
            QCOMPARE(ground_truth_values[item_no], client_value);
        }

        for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
            float ground_truth = rand() * 2.f / (float) RAND_MAX - 1.f;
            _updated = false;
            tweak_set_scalar_float(tweak_ids[item_no], ground_truth);
            QVariant value = waitChange();
            QCOMPARE(ground_truth, value.toFloat());
        }

        for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
            float ground_truth = rand() * 2.f / (float) RAND_MAX - 1.f;
            _updated = false;
            tweak_set_scalar_float(tweak_ids[item_no], ground_truth);
            QVariant value = waitChange();
            QCOMPARE(ground_truth, value.toFloat());
        }

        for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
            QVariant ground_truth = QVariant::fromValue(rand() * 2.f / (float) RAND_MAX - 1.f);
            QModelIndex index = tweakApplication->index(item_no);
            _serverUpdated = false;
            tweakApplication->setData(index, ground_truth, TweakApplication::Roles::ValueRole);
            float value = waitServerUpdate();
            QCOMPARE(ground_truth.toFloat(), value);
        }

        {
            std::vector<uint32_t> indices;
            indices.resize(TEST_ITEM_COUNT);
            for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
                indices[item_no] = item_no;
            }
            std::random_device rd;
            std::mt19937 urbg(rd());
            std::shuffle(indices.begin(), indices.end(), urbg);
            std::vector<uint32_t> indices1 = indices;

             for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
                _removed = false;
                tweak_remove(tweak_ids[indices[item_no]]);
                struct row_interval row_interval = waitRemove();
                QCOMPARE(row_interval.start, (int)indices1[item_no]);
                QCOMPARE(row_interval.end, row_interval.start);
                uint32_t cutoff = indices1[item_no];
                for (uint32_t ix = item_no; ix < TEST_ITEM_COUNT; ix++) {
                    if (indices1[ix] > cutoff) {
                        --indices1[ix];
                    }
                }
            }
        }

        for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
            char uri[100];
            snprintf(uri, sizeof(uri), "item_%d", item_no);
            ground_truth_values[item_no] = rand() * 2.f / (float) RAND_MAX - 1.f;
            tweak_ids[item_no] = tweak_add_scalar_float(uri, uri, uri, ground_truth_values[item_no]);
        }

        for (uint32_t item_no = 0; item_no < TEST_ITEM_COUNT; item_no++) {
            float ground_truth = rand() * 2.f / (float) RAND_MAX - 1.f;
            _updated = false;
            tweak_set_scalar_float(tweak_ids[item_no], ground_truth);
            QVariant value = waitChange();
            QCOMPARE(ground_truth, value.toFloat());
        }

        tweak_finalize_library();
    }
};
} // namespace tweak2

#include "TweakQmlApp.test.moc"
QTEST_MAIN(tweak2::TestTweakApplication)
