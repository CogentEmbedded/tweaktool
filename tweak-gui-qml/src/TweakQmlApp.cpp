/**
 * @file tweakqmlapp.cpp
 * @ingroup GUI
 *
 * @brief Tweak QML Application Model.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#include "TweakQmlApp.hpp"
#include "TweakQmlApp_p.hpp"

#include "QTweakVariant.hpp"

#include <QDebug>
#include <algorithm>

namespace tweak2
{

TweakApplication::TweakApplication(QObject *parent, Qt::ConnectionType connectionType)
    : QAbstractListModel(parent), d_ptr(new TweakApplicationPrivate(this, connectionType))
{
    Q_D(TweakApplication);

    for (const auto &f : d->readFavorites())
    {
        addFavorite(f);
    }
}

int TweakApplication::rowCount(const QModelIndex &parent) const
{
    Q_D(const TweakApplication);
    Q_UNUSED(parent);

    int result = (int) d->tweakControlIdList.size();

    return result;
}

QVariant TweakApplication::data(const QModelIndex &index, int role) const
{
    Q_D(const TweakApplication);

    if (!index.isValid() || index.column() != 0)
    {
        return QVariant();
    }

    if (!(index.row() >= 0 && index.row() < d->tweakControlIdList.size()))
    {
        return QVariant();
    }

    const TweakControlId id = d->tweakControlIdList[index.row()];
    auto predicate = [&id](const ConnectionItem& arg) {
        return id.connectionId == arg.connectionId;
    };

    tweak_app_client_context clientContext;
    QString name;
    {
        QReadLocker locker(&d->lock);
        auto itr = std::find_if(d->connectionIdList.begin(), d->connectionIdList.end(), predicate);
        if (itr == d->connectionIdList.end()) {
            return QVariant();
        }
        clientContext = itr->clientContext;
        name = itr->name;
    }

    QVariant result;
    tweak_app_item_snapshot *sn = tweak_app_item_get_snapshot(clientContext, id.tweakId);
    if (sn)
    {
        const QString uri = "/" + name + from_tweak_string(&sn->uri);
        switch (role)
        {
        case Qt::DisplayRole:
            result = from_tweak_variant(&sn->current_value).toString();
            break;

        case ValueRole:
            result = from_tweak_variant(&sn->current_value);
            break;

        case UriRole:
            result = uri;
            break;

        case DefaultValueRole:
            result = from_tweak_variant(&sn->default_value);
            break;

        case DescriptionRole:
        case Qt::ToolTipRole:
            result = from_tweak_string(&sn->description);
            break;

        case MetaRole: 
            result = QVariant::fromValue(d->metadataParser.parse(from_tweak_string(&sn->meta)));
            break;

        case isFavoriteRole:
            result = isFavorite(uri);
            break;

        default:
            qWarning("Unknown role requested in TweakApplication::data(): %d", role);
            break;
        }

        tweak_app_release_snapshot(clientContext, sn);
        sn = nullptr;
    }

    return result;
}

bool TweakApplication::setData(const QModelIndex &index, const QVariant &value, int role) {
    Q_D(const TweakApplication);

    if (role != ValueRole) {
        return false;
    }

    if (!index.isValid() || index.column() != 0)
    {
        return false;
    }

    if (index.row() >= d->tweakControlIdList.size())
    {
        return false;
    }

    const TweakControlId id = d->tweakControlIdList[index.row()];
    auto predicate = [&id](const ConnectionItem& arg) {
        return id.connectionId == arg.connectionId;
    };
    tweak_app_client_context clientContext;
    {
        QReadLocker locker(&d->lock);
        auto itr = std::find_if(d->connectionIdList.begin(), d->connectionIdList.end(), predicate);
        if (itr == d->connectionIdList.end()) {
            return false;
        }
        clientContext = itr->clientContext;
    }
    tweak_variant_type type = tweak_app_item_get_type(clientContext, id.tweakId);
    tweak_variant tweak_variant_value = TWEAK_VARIANT_INIT_EMPTY;
    to_tweak_variant(&tweak_variant_value, type, value);
    tweak_app_error_code error_code = tweak_app_item_replace_current_value(clientContext, id.tweakId, &tweak_variant_value);

    emit dataChanged(index, index, {role});

    return error_code == TWEAK_APP_SUCCESS;
}

QHash<int, QByteArray> TweakApplication::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
        {Qt::ToolTipRole, "toolTip"},
        {Qt::EditRole, "edit"},
        {UriRole, "uri"},
        {ValueRole, "tweakValue"},
        {DefaultValueRole, "defaultValue"},
        {DescriptionRole, "description"},
        {MetaRole, "meta"},
        {isFavoriteRole, "isFavorite"},
    };
}

QVariant TweakApplication::get(ConnectionId connectionId, const QUrl &uri) const {
    return get(connectionId, uri.toString());
}

QVariant TweakApplication::get(ConnectionId connectionId, const QString &path) const {
    Q_D(const TweakApplication);
    QModelIndex modelIndex = d->indexByUri(connectionId, path);
    int row = modelIndex.row();
    if (row < 0) {
        return QVariant();
    }

    return data(modelIndex, ValueRole);
}

qint64 TweakApplication::addClient(QString name, QString contextType, QString params, QString uri) {
    Q_D(TweakApplication);
    return d->addClient(name, contextType, params, uri);
}

void TweakApplication::removeClient(qint64 clientConnectionId) {
    Q_D(TweakApplication);
    beginResetModel();
    d->removeClient(clientConnectionId);
    endResetModel();
}

void TweakApplication::addFavorite(const QString &uri)
{
    Q_D(TweakApplication);

    favorites.insert(uri);

    emit favoritesRegExChanged();

    d->saveFavorites(favorites.values());

    /*.. TODO emit events by uri only */
    emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 0), {isFavoriteRole});
}

void TweakApplication::removeFavorite(const QString &uri)
{
    Q_D(TweakApplication);

    if (favorites.remove(uri))
    {
        emit favoritesRegExChanged();

        d->saveFavorites(favorites.values());

        /*.. TODO emit events by uri only */
        emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 0), {isFavoriteRole});
    }
}

void TweakApplication::clearFavorites()
{
    Q_D(TweakApplication);

    favorites.clear();
    emit favoritesRegExChanged();

    d->saveFavorites(favorites.values());
}

bool TweakApplication::isFavorite(const QString &uri) const
{
    return favorites.contains(uri);
}

void TweakApplication::currentValueChanged(quint64 connection_id, quint64 tweak_id)
{
    Q_D(TweakApplication);
    d->currentValueChangedImpl(connection_id, tweak_id);
}
void TweakApplication::statusChanged(quint64 connection_id, bool is_connected)
{
    Q_D(TweakApplication);
    d->statusChangedImpl(connection_id, is_connected);
}
void TweakApplication::newItem(quint64 connection_id, quint64 tweak_id, QString uri)
{
    Q_D(TweakApplication);
    d->newItemImpl(connection_id, tweak_id, uri);
}

void TweakApplication::itemRemoved(quint64 connection_id, quint64 tweak_id)
{
    Q_D(TweakApplication);
    d->itemRemovedImpl(connection_id, tweak_id);
}

TweakApplicationPrivate::TweakApplicationPrivate(TweakApplication *application, Qt::ConnectionType connectionType)
    : q_ptr(application)
    , seed(0)
    , treeModel(application)
    , metadataParser(application)
    , connectionType(connectionType)
{
}

QModelIndex TweakApplicationPrivate::indexByUri(ConnectionId connectionId, QString uri) const {
    Q_Q(const TweakApplication);
    
    tweak_app_client_context clientContext;
    {
        QReadLocker locker(&lock);
        auto connIdPredicate = [connectionId](const ConnectionItem& arg) {
            return connectionId == arg.connectionId;
        };
        auto connectionIdListItr = std::find_if(connectionIdList.begin(),
                                                connectionIdList.end(), connIdPredicate);
        if (connectionIdListItr == connectionIdList.end()) {
            return QModelIndex();
        }
        clientContext = connectionIdListItr->clientContext;
    }
    tweak_id tweak_id = tweak_app_find_id(clientContext, uri.toStdString().c_str());
    if (tweak_id == TWEAK_INVALID_ID) {
        return QModelIndex();
    }

    TweakControlId tweakControlId(connectionId, tweak_id);
    auto tweakControlIdListItr = std::find(tweakControlIdList.begin(),
                                           tweakControlIdList.end(), tweakControlId);
    if (tweakControlIdListItr == tweakControlIdList.end()) {
        return QModelIndex();
    }

    return q->index(static_cast<int>(tweakControlIdListItr - tweakControlIdList.begin()));
}

TweakTreeModel *TweakApplication::getTreeModel()
{
    Q_D(TweakApplication);

    return &d->treeModel;
}

TweakMetadataParser *TweakApplication::getMetadataParser()
{
    Q_D(TweakApplication);
    return &d->metadataParser;
}

QString TweakApplication::getfavoritesRegEx() const
{
    QString regex;
    for (const auto &f : favorites)
    {
        if (!regex.isEmpty())
        {
            regex += "|";
        }

        regex += "^" + QRegExp::escape(f) + "$";
    }

    if (regex.isEmpty())
    {
        /*.. filter none */
        regex = "^$";
    }

    return regex;
}

Qt::ItemFlags TweakApplication::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}

ConnectionId TweakApplicationPrivate::addClient(QString name, QString contextType, QString params, QString uri) {
    auto predicate = [&contextType, &params, &uri](const ConnectionItem& arg) {
        return contextType == arg.contextType
                && params == arg.params
                && uri == arg.uri;
    };
    
    {
        QReadLocker locker(&lock);
        auto connectionIdListItr = std::find_if(connectionIdList.begin(), connectionIdList.end(), predicate);
        if (connectionIdListItr != connectionIdList.end()) {
            qWarning() << "Attmept to add a connection twice";
            return connectionIdListItr->connectionId;
        }
    }

    tweak_app_client_callbacks client_callbacks = {
        .cookie = this,
        .on_connection_status_changed = &TweakApplicationPrivate::statusChangedAdapter,
        .on_new_item = &TweakApplicationPrivate::newItemAdapter,
        .on_current_value_changed = &TweakApplicationPrivate::currentValueChangedAdapter,
        .on_item_removed = &TweakApplicationPrivate::itemRemovedAdapter
    };

    {
        QWriteLocker locker(&lock);

        tweak_app_client_context clientContext =
                tweak_app_create_client_context(contextType.toStdString().c_str(),
                                                params.toStdString().c_str(),
                                                uri.toStdString().c_str(),
                                                &client_callbacks);

        if (clientContext) {
            ConnectionId connectionId = ++seed;
            ConnectionItem connectionItem = {
                .name = name,
                .connectionId = connectionId,
                .contextType = contextType,
                .params = params,
                .uri = uri,
                .clientContext = clientContext,
            };

            connectionIdList.push_back(connectionItem);
            return connectionId;
        } else {
            qWarning() << "Failed to create clientContext";
            return InvalidClientConnectionId;
        }
    }
}

void TweakApplicationPrivate::removeClient(ConnectionId connectionId) {
    Q_Q(TweakApplication);

    auto connectionIdListPredicate = [connectionId](const ConnectionItem& arg) -> bool {
        return connectionId == arg.connectionId;
    };
    tweak_app_client_context clientContext = NULL;
    {
        QReadLocker locker(&lock);
        auto connectionIdListItr = std::find_if(connectionIdList.begin(),
            connectionIdList.end(), connectionIdListPredicate);
        if (connectionIdListItr != connectionIdList.end()) {
            clientContext = connectionIdListItr->clientContext;
        } else {
            qWarning() << "Attempt to remove a connection that does not exist";
        }
    }
    if (clientContext) {
        tweak_app_destroy_context(clientContext);
        {
            QWriteLocker locker(&lock);
            connectionIdList.erase(
                std::remove_if(connectionIdList.begin(), connectionIdList.end(), connectionIdListPredicate),
                connectionIdList.end()
            );
        }
    }
}

QStringList TweakApplicationPrivate::readFavorites()
{
    settings.beginGroup("favorites");
    QStringList list = settings.value("uris").toStringList();
    settings.endGroup();

    return list;
}

void TweakApplicationPrivate::saveFavorites(const QStringList &uris)
{
    settings.beginGroup("favorites");
    settings.setValue("uris", uris);
    settings.endGroup();
}

TweakApplicationPrivate::~TweakApplicationPrivate() {
    QWriteLocker locker(&lock);
    for (auto itr = connectionIdList.begin(); itr != connectionIdList.end(); ++itr) {
        tweak_app_destroy_context(itr->clientContext);
    }
    connectionIdList.clear();
}

ConnectionId TweakApplicationPrivate::appContextToConnId(tweak_app_context context) {
    QReadLocker locker(&lock);
    for (auto itr = connectionIdList.begin(); itr != connectionIdList.end(); ++itr) {
        if (itr->clientContext == context) {
            return itr->connectionId;
        }
    }
    return InvalidClientConnectionId;
}

void TweakApplicationPrivate::statusChangedImpl(quint64 connection_id, bool is_connected) {
}

void TweakApplicationPrivate::newItemImpl(quint64 connection_id, quint64 tweak_id, QString uri) {
    Q_Q(TweakApplication);

    TweakControlId tweakControlId(connection_id, tweak_id);

    if (!tweakControlId.isValid()) {
        qWarning() << "Invalid tweak control id received for newItem";
        return;
    }

    QString path = "";

    auto predicate = [&tweakControlId](const ConnectionItem& arg) {
        return tweakControlId.connectionId == arg.connectionId;
    };
    tweak_app_client_context clientContext = NULL;
    QString name;
    {
        QReadLocker locker(&lock); /*.. needed ? */
        auto itr = std::find_if(connectionIdList.begin(), connectionIdList.end(), predicate);
        if (itr != connectionIdList.end()) {
            clientContext = itr->clientContext;
            name = itr->name;
        }
    }

    if (!clientContext) {
        qWarning() << "clientContext not found during newItem";
        return;
    }

    path = "/" + name + uri;

    size_t pos = tweakControlIdList.size();

    tweakControlIdList.push_back(tweakControlId);
    /*.. no cache cleaning is needed because new item index is always greater
         * than everything in the cache and existing indices are not touched */

    treeModel.newItem(path, tweakControlId);

    q->beginInsertRows(QModelIndex(), pos, pos);
    q->endInsertRows();
}

void TweakApplicationPrivate::currentValueChangedImpl(quint64 connection_id, quint64 tweak_id)
{
    Q_Q(TweakApplication);
    bool emitEvent = false;
    TweakControlIdListIndex list_pos = -1;
    TweakControlId tweakControlId(connection_id, tweak_id);

    auto itr = tweakControlIdCache.find(tweakControlId);
    if (itr != tweakControlIdCache.end()) {
        list_pos = *itr;
        emitEvent = true;
    } else {
        auto itr = std::find(tweakControlIdList.begin(), tweakControlIdList.end(), tweakControlId);
        if (itr != tweakControlIdList.end()) {
            list_pos = std::distance(tweakControlIdList.begin(), itr);
            tweakControlIdCache[tweakControlId] = list_pos;
            emitEvent = true;
        }
    }
    if (emitEvent) {
        QModelIndex modelIndex = q->index(list_pos);
        emit q->dataChanged(modelIndex, modelIndex, {TweakApplication::ValueRole});
    } else {
        qWarning() << "Spurious current value change for a tweak that is not found.";
    }
}

void TweakApplicationPrivate::itemRemovedImpl(quint64 connection_id, quint64 tweak_id) {
    Q_Q(TweakApplication);

    TweakControlId tweakControlId(connection_id, tweak_id);
    if (!tweakControlId.isValid()) {
        qWarning() << "Removing an item with invalid id";
        return;
    }

    auto itr = std::find(tweakControlIdList.begin(), tweakControlIdList.end(), tweakControlId);
    if (itr != tweakControlIdList.end()) {
        size_t pos = itr - tweakControlIdList.begin();
        tweakControlIdCache.clear();
        tweakControlIdList.erase(itr);
        treeModel.itemRemoved(tweakControlId);

        q->beginRemoveRows(QModelIndex(), pos, pos);
        q->endRemoveRows();
    } else {
        qWarning() << "Attempt to remove an item that was not found";
    }

}

void TweakApplicationPrivate::statusChangedAdapter(tweak_app_context context, bool is_connected, void *cookie)
{
    TweakApplicationPrivate* tweakApplicationPrivate = (TweakApplicationPrivate*) cookie;
    Q_ASSERT(tweakApplicationPrivate != NULL);
    ConnectionId connectionId = tweakApplicationPrivate->appContextToConnId(context);
    if (connectionId == InvalidClientConnectionId)
        return;

    QMetaObject::invokeMethod(tweakApplicationPrivate->q_ptr, "statusChanged",
                              tweakApplicationPrivate->connectionType,
                              Q_ARG(quint64, connectionId),
                              Q_ARG(bool, is_connected));
}

void TweakApplicationPrivate::newItemAdapter(tweak_app_context context, tweak_id id, void *cookie)
{
    TweakApplicationPrivate* tweakApplicationPrivate = (TweakApplicationPrivate*) cookie;
    Q_ASSERT(tweakApplicationPrivate != NULL);
    ConnectionId connectionId = tweakApplicationPrivate->appContextToConnId(context);
    if (connectionId == InvalidClientConnectionId)
        return;

    tweak_app_item_snapshot *sn = tweak_app_item_get_snapshot(context, id);
    Q_ASSERT(sn != NULL);
    QString uri = from_tweak_string(&sn->uri);
    tweak_app_release_snapshot(context, sn);

    QMetaObject::invokeMethod(tweakApplicationPrivate->q_ptr, "newItem",
                              tweakApplicationPrivate->connectionType,
                              Q_ARG(quint64, connectionId),
                              Q_ARG(quint64, id),
                              Q_ARG(QString, uri));
}

void TweakApplicationPrivate::currentValueChangedAdapter(tweak_app_context context, tweak_id id,
                                                         tweak_variant* value, void *cookie)
{
    TweakApplicationPrivate* tweakApplicationPrivate = (TweakApplicationPrivate*) cookie;
    Q_ASSERT(tweakApplicationPrivate != NULL);
    ConnectionId connectionId = tweakApplicationPrivate->appContextToConnId(context);
    if (connectionId == InvalidClientConnectionId)
        return;

    QMetaObject::invokeMethod(tweakApplicationPrivate->q_ptr, "currentValueChanged",
                              tweakApplicationPrivate->connectionType,
                              Q_ARG(quint64, connectionId),
                              Q_ARG(quint64, id));
}

void TweakApplicationPrivate::itemRemovedAdapter(tweak_app_context context, tweak_id id, void *cookie) {
    TweakApplicationPrivate* tweakApplicationPrivate = (TweakApplicationPrivate*) cookie;
    Q_ASSERT(tweakApplicationPrivate != NULL);
    ConnectionId connectionId = tweakApplicationPrivate->appContextToConnId(context);
    if (connectionId == InvalidClientConnectionId)
        return;

    QMetaObject::invokeMethod(tweakApplicationPrivate->q_ptr, "itemRemoved",
                              tweakApplicationPrivate->connectionType,
                              Q_ARG(quint64, connectionId),
                              Q_ARG(quint64, id));
}

} // namespace tweak2
