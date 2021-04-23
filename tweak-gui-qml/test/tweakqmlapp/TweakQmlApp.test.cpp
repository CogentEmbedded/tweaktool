/**
 * @file TweakQmlApp.test.cpp
 * @ingroup GUI
 *
 * @brief test suite for TweakQml component.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#include <QtTest/QtTest>

#include "TweakQmlApp.hpp"
#include <tweak2/tweak2.h>

#include <stdexcept>
#include <vector>
#include <errno.h>
#include <pthread.h>

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

  pthread_mutex_t _lock;
  pthread_cond_t _cond;
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
    pthread_mutex_lock(&_lock);
    struct row_interval tmp = { start, end };
    std::swap(_row_interval, tmp);
    _inserted = true;
    pthread_cond_broadcast(&_cond);
    pthread_mutex_unlock(&_lock);
  }

  void dataChanged(TweakApplication *tweakApplication,
    const QModelIndex &topLeft, const QModelIndex &bottomRight,
    const QVector<int> &roles)
  {
    (void)bottomRight;
    (void)roles;
    pthread_mutex_lock(&_lock);
    _value = tweakApplication->data(topLeft, TweakApplication::ValueRole);
    _updated = true;
    pthread_cond_broadcast(&_cond);
    pthread_mutex_unlock(&_lock);
  }

  void rowsAboutToBeRemoved(TweakApplication *tweakApplication,
    const QModelIndex &parent, int start, int end)
  {
    (void)tweakApplication;
    (void)parent;
    pthread_mutex_lock(&_lock);
    struct row_interval tmp = { start, end };
    std::swap(_row_interval, tmp);
    _removed = true;
    pthread_cond_broadcast(&_cond);
    pthread_mutex_unlock(&_lock);
  }

  QVariant waitChange() {
    QVariant result;
    pthread_mutex_lock(&_lock);
    while (!_updated) {
        pthread_cond_wait(&_cond, &_lock);
    }
    result.swap(_value);
    pthread_mutex_unlock(&_lock);
    return result;
  }

  struct row_interval waitInsert() {
    struct row_interval result = { -1, -1};
    pthread_mutex_lock(&_lock);
    while (!_inserted) {
        pthread_cond_wait(&_cond, &_lock);
    }
    std::swap(_row_interval, result);
    pthread_mutex_unlock(&_lock);
    return result;
  }
  struct row_interval waitRemove() {
    struct row_interval result = { -1, -1};
    pthread_mutex_lock(&_lock);
    while (!_removed) {
        pthread_cond_wait(&_cond, &_lock);
    }
    std::swap(_row_interval, result);
    pthread_mutex_unlock(&_lock);
    return result;
  }

  void itemChangeListener(tweak_id tweak_id) {
    pthread_mutex_lock(&_lock);
    _serverValue = tweak_get_scalar_float(tweak_id);
    _serverUpdated = true;
    pthread_cond_broadcast(&_cond);
    pthread_mutex_unlock(&_lock);
  }

  static void itemChangeListenerAddapter(tweak_id tweak_id, void* cookie) {
    TestTweakApplication* testTweakApplication = static_cast<TestTweakApplication*>(cookie);
    testTweakApplication->itemChangeListener(tweak_id);
  }

  float waitServerUpdate() {
    float result;
    pthread_mutex_lock(&_lock);
    while (!_serverUpdated) {
        pthread_cond_wait(&_cond, &_lock);
    }
    std::swap(_serverValue, result);
    pthread_mutex_unlock(&_lock);
    return result;
  }

  private slots:
    void sanityTest() {
        pthread_mutex_init(&_lock, NULL);
        pthread_cond_init(&_cond, NULL);

        tweak_initialize_library("nng", "role=server", "tcp://0.0.0.0:7777/");

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

        ConnectionId connectionId = tweakApplication->addClient("mock", "nng", "role=client", "tcp://0.0.0.0:7777/");
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
            std::random_shuffle(indices.begin(), indices.end());
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

        pthread_cond_destroy(&_cond);
        pthread_mutex_destroy(&_lock);
    }
};
} // namespace tweak2

#include "TweakQmlApp.test.moc"
QTEST_MAIN(tweak2::TestTweakApplication)
