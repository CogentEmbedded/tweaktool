/**
 * @file TweakMetadataParser.cpp
 * @ingroup GUI
 *
 * @brief Metdata parser for Tweak QML GUI.
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

#include "TweakMetadataParser.hpp"
#include "QTweakVariant.hpp"

#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace tweak2
{

TweakMetadata::TweakMetadata(QObject *parent)
    : QObject(parent)
    , m_metadata(nullptr)
    , m_options(nullptr)
{}

TweakMetadata::TweakMetadata(tweak_variant_type item_type,
                             tweak_metadata metadata,
                             QString json,
                             std::unique_ptr<TweakFixedPointInfo> info,
                             QObject *parent)
    : QObject(parent)
    , m_item_type(item_type)
    , m_metadata(metadata)
    , m_options(nullptr)
    , m_json(json)
    , m_info(std::move(info))
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
    case TWEAK_METADATA_CONTROL_TABLE:
        return tweak2::TweakMetadata::ControlType::Table;
    case TWEAK_METADATA_CONTROL_EDITBOX:
        return tweak2::TweakMetadata::ControlType::Editbox;
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

QString TweakMetadata::getUnit() const {
    return QString(tweak_variant_string_c_str(tweak_metadata_get_unit(m_metadata)));
}

QString TweakMetadata::getJson() const {
    return m_json;
}

OptionsModel* TweakMetadata::getOptions() const {
    return m_options;
}

tweak_variant_type TweakMetadata::getTweakType() const
{
    return m_item_type;
}

TweakFixedPointInfo *TweakMetadata::getFixedPointInfo() const
{
    return m_info.get();
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
    std::unique_ptr<TweakFixedPointInfo> info = nullptr;

    /*.. parsing of extra fields in the metadata must be done manually */
    QJsonParseError error;
    QJsonDocument doc =  QJsonDocument::fromJson(meta.toUtf8(), &error);
    QJsonObject obj = doc.object();
    if (obj.contains("fixed_point") && obj["fixed_point"].isString())
    {
        QString type_name = obj["fixed_point"].toString();
        static QRegularExpression reg("^(L?)([us]?)(\\d+)(q|a|sm|z)(\\d+)$");

        auto match = reg.match(type_name);

        if (match.hasMatch())
        {
            bool is_signed = (match.captured(1) == "s");
            unsigned integer_bits = match.captured(2).toULong();
            QString fp_type = match.captured(3);
            unsigned fraction_bits = match.captured(4).toULong();

            TweakFixedPointInfo::FixedPointType fpt;
            if (fp_type == "q")
            {
                fpt = TweakFixedPointInfo::FixedPointType::TwosComplement;
            }
            else if (fp_type == "a")
            {
                fpt = TweakFixedPointInfo::FixedPointType::Apical;
            }
            else if (fp_type == "sm")
            {
                fpt = TweakFixedPointInfo::FixedPointType::SignMantissa;
            }
            else if (fp_type == "z")
            {
                fpt = TweakFixedPointInfo::FixedPointType::OnesComplement;
            }
            else
            {
                //  Q_UNREACHABLE();
                qWarning("Unknown fp_type: %s", fp_type.toStdString().c_str());
            }

            info = std::make_unique<TweakFixedPointInfo>(is_signed, fpt, integer_bits, fraction_bits);
        }
    }

    return new TweakMetadata(item_type,
                             tweak_metadata_create(item_type, 1, meta.toUtf8().constData()),
                             meta,
                             std::move(info),
                             nullptr);
}

TweakFixedPointInfo::TweakFixedPointInfo(bool is_signed,
                                         TweakFixedPointInfo::FixedPointType fpt,
                                         unsigned integer_bits,
                                         unsigned fraction_bits)
    : m_is_signed(is_signed)
    , m_fpt(fpt)
    , m_integer_bits(integer_bits)
    , m_fraction_bits(fraction_bits)
{

}

TweakFixedPointInfo::~TweakFixedPointInfo()
{

}

bool TweakFixedPointInfo::getIsSigned() const
{
    return m_is_signed;
}

TweakFixedPointInfo::FixedPointType TweakFixedPointInfo::getFpt() const
{
    return m_fpt;
}

unsigned TweakFixedPointInfo::getIntegerBits() const
{
    return m_integer_bits;
}

unsigned TweakFixedPointInfo::getFractionBits() const
{
    return m_fraction_bits;
}

QVariant TweakFixedPointInfo::getMin() const
{
    return 0;
}

QVariant TweakFixedPointInfo::getMax() const
{
    return 0;
}

}
