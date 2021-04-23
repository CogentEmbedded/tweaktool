/**
 * @file tweakqmlapp_p.hpp
 * @ingroup GUI
 *
 * @brief Private part of Tweak QML Application Model.
 *
 * @copyright 2020-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#ifndef TWEAK_QML_APP_P_H_INCLUDED
#define TWEAK_QML_APP_P_H_INCLUDED

#include "TweakTreeModel.hpp"
#include <tweak2/appclient.h>
#include "TweakQmlApp.hpp"

#include <QScopedPointer>
#include <QHash>
#include <QReadWriteLock>
#include <QSettings>
#include <vector>

namespace tweak2
{
class ConnectionItem {
    QString name;
    ConnectionId connectionId;
    QString contextType;
    QString params;
    QString uri;
    tweak_app_client_context clientContext;
public:
    ConnectionItem() = delete;
    ConnectionItem(const ConnectionItem&) = delete;
    ConnectionItem& operator=(const ConnectionItem&) = delete;
    ConnectionItem& operator=(ConnectionItem&& connectionItem);
    ConnectionItem(ConnectionItem&& connectionItem);
    ConnectionItem(QString name, ConnectionId connectionId, QString contextType,
                   QString params, QString uri, tweak_app_client_context clientContext);
    QString getName() const;
    ConnectionId getConnectionId() const;
    QString getContextType() const;
    QString getParams() const;
    QString getUri() const;
    tweak_app_client_context getClientContext() const;
    tweak_app_client_context releaseClientContext();
    ~ConnectionItem();
};

using ConnectionIdList = std::vector<ConnectionItem>;

using TweakControlIdList = std::vector<TweakControlId>;

using TweakControlIdListIndex = typename TweakControlIdList::difference_type;

using TweakControlIdCache = QHash<TweakControlId, TweakControlIdListIndex>;

class TweakApplicationPrivate
{
    Q_DISABLE_COPY(TweakApplicationPrivate)
    TweakApplication *const q_ptr;
    ConnectionId seed;
    mutable QReadWriteLock lock;
    Q_DECLARE_PUBLIC(TweakApplication)

    static void statusChangedAdapter(tweak_app_context context, bool is_connected, void *cookie);

    static void newItemAdapter(tweak_app_context context, tweak_id id, void *cookie);

    static void currentValueChangedAdapter(tweak_app_context context, tweak_id id,
        tweak_variant* value, void *cookie);

    static void itemRemovedAdapter(tweak_app_context context, tweak_id id, void *cookie);

    ConnectionId appContextToConnId(tweak_app_context context);

    void currentValueChangedImpl(quint64 connection_id, quint64 tweak_id);

    void statusChangedImpl(quint64 connection_id, bool is_connected);

    void newItemImpl(quint64 connection_id, quint64 tweak_id, QString uri);

    void itemRemovedImpl(quint64 connection_id, quint64 tweak_id);

  public:
    explicit TweakApplicationPrivate(TweakApplication *application, Qt::ConnectionType connectionType);

    ConnectionId addClient(QString name, QString contextType, QString params, QString uri);

    void removeClient(ConnectionId clientConnectionId);

    ConnectionIdList connectionIdList;

    TweakControlIdList tweakControlIdList;

    TweakControlIdCache tweakControlIdCache;

    TweakTreeModel treeModel;

    TweakMetadataParser metadataParser;

    Qt::ConnectionType connectionType;

    QSettings settings;

    QStringList readFavorites();
    void saveFavorites(const QStringList &uris);

    QModelIndex indexByUri(ConnectionId connectionId, QString uri) const;
};

} // namespace tweak2

#endif /* TWEAK_QML_APP_P_H_INCLUDED */
