/**
 * @file tweakmetadataparser.hpp
 * @ingroup GUI
 *
 * @brief Metdata parser for Tweak QML GUI.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#include "TweakMetadataParser.hpp"
#include "QTweakVariant.hpp"

#include <QTextStream>

namespace tweak2
{

TweakMetadata::TweakMetadata(QObject *parent)
    : QObject(parent)
    , m_metadata(nullptr)
    , m_options(nullptr)
{}

TweakMetadata::TweakMetadata(tweak_metadata metadata, QObject *parent)
    : QObject(parent)
    , m_metadata(metadata)
    , m_options(nullptr)
{
    tweak_metadata_options options = tweak_metadata_get_options(m_metadata);
    if (options) {
        m_options = new OptionsModel(options, this);
    }
}

TweakMetadata::~TweakMetadata() {
    tweak_metadata_destroy(m_metadata);
}

TweakMetadata::ControlType TweakMetadata::getControlType() const {
    switch (tweak_metadata_get_control_type(m_metadata)) {
    case TWEAK_METADATA_CONTROL_CHECKBOX:
        return tweak2::TweakMetadata::ControlType::Checkbox;
    case TWEAK_METADATA_CONTROL_SPINBOX:
        return tweak2::TweakMetadata::ControlType::Spinbox;
    case TWEAK_METADATA_CONTROL_SLIDER:
        return tweak2::TweakMetadata::ControlType::Slider;
    case TWEAK_METADATA_CONTROL_COMBOBOX:
        return tweak2::TweakMetadata::ControlType::Combobox;
    case TWEAK_METADATA_CONTROL_BUTTON:
        return tweak2::TweakMetadata::ControlType::Button;
    default:
        break;
    }
    return tweak2::TweakMetadata::ControlType::Unknown;
}

QVariant TweakMetadata::getMin() const {
    return from_tweak_variant(tweak_metadata_get_min(m_metadata));
}

QVariant TweakMetadata::getMax() const {
    return from_tweak_variant(tweak_metadata_get_max(m_metadata));
}

bool TweakMetadata::getReadonly() const {
    return tweak_metadata_get_readonly(m_metadata);
}

quint32 TweakMetadata::getDecimals() const {
    return static_cast<quint32>(tweak_metadata_get_decimals(m_metadata));
}

QVariant TweakMetadata::getStep() const {
    return from_tweak_variant(tweak_metadata_get_step(m_metadata));
}

QString TweakMetadata::getCaption() const {
    return QString(tweak_variant_string_c_str(tweak_metadata_get_caption(m_metadata)));
}

OptionsModel* TweakMetadata::getOptions() const {
    return m_options;
}

OptionsModel::OptionsModel(QObject *parent)
    : QAbstractListModel(parent)
{}

OptionsModel::OptionsModel(tweak_metadata_options options, QObject *parent)
    : QAbstractListModel(parent)
    , m_options(options)
{}

int OptionsModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return static_cast<int>(tweak_metadata_get_enum_size(m_options));
}


QHash<int, QByteArray> OptionsModel::roleNames() const {
    return {
        { OptionsRoles::TextRole, "text" },
        { OptionsRoles::ValueRole, "value" }
    };
}


QVariant OptionsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.column() != 0)
    {
        return QVariant();
    }

    if (!(index.row() >= 0 && index.row() < static_cast<int>(tweak_metadata_get_enum_size(m_options))))
    {
        return QVariant();
    }

    QVariant result;
    switch (role)
    {
    case OptionsRoles::ValueRole:
        result = from_tweak_variant(tweak_metadata_get_enum_value(m_options, index.row()));
        break;

    case OptionsRoles::TextRole:
        result = QString(tweak_variant_string_c_str(tweak_metadata_get_enum_text(m_options, index.row())));
        break;

    default:
        qWarning("Unknown role requested in TweakApplication::data(): %d", role);
        break;
    }

    return result;
}

QVariant OptionsModel::get(int index) const {
    int size = static_cast<int>(tweak_metadata_get_enum_size(m_options));
    return index < size
        ? from_tweak_variant(tweak_metadata_get_enum_value(m_options, index))
        : QVariant();
}

int OptionsModel::find(QVariant value) const {
    int size = static_cast<int>(tweak_metadata_get_enum_size(m_options));
    for (int ix = 0; ix < size; ix++) {
        QVariant rowValue = from_tweak_variant(tweak_metadata_get_enum_value(m_options, ix));
        if (value == rowValue) {
            return ix;
        }
    }
    return -1;
}

TweakMetadataParser::TweakMetadataParser(QObject *parent)
    : QObject(parent)
{}

TweakMetadata* TweakMetadataParser::parse(tweak_variant_type item_type, QString meta) const {
    return new TweakMetadata(tweak_metadata_create(item_type, meta.toUtf8().constData()), nullptr);
}

}
