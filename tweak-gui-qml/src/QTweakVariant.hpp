/**
 * @file qtweakvariant.hpp
 * @ingroup GUI
 *
 * @brief Qt binding for Tweak Variant.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#ifndef QTWEAKVARIANT_H
#define QTWEAKVARIANT_H

#include <QString>
#include <QVariant>

#include <tweak2/string.h>
#include <tweak2/variant.h>

namespace tweak2
{

static inline QString from_tweak_string(const tweak_variant_string *arg)
{
    return QString(tweak_variant_string_c_str(arg));
}

static inline void to_tweak_string(tweak_variant_string *dest, QString src)
{
    QByteArray src0 = src.toUtf8();
    tweak_variant_assign_string(dest, src0.constData());
}

static inline QVariant from_tweak_variant(const tweak_variant *arg)
{
    Q_ASSERT(arg);

    switch (arg->type)
    {
    case TWEAK_VARIANT_TYPE_NULL:
        return QVariant();
    case TWEAK_VARIANT_TYPE_BOOL:
        return QVariant(arg->value.b);
    case TWEAK_VARIANT_TYPE_SINT8:
        return QVariant(arg->value.sint8);
    case TWEAK_VARIANT_TYPE_SINT16:
        return QVariant(arg->value.sint16);
    case TWEAK_VARIANT_TYPE_SINT32:
        return QVariant(arg->value.sint32);
    case TWEAK_VARIANT_TYPE_SINT64:
        return QVariant(static_cast<qlonglong>(arg->value.sint64));
    case TWEAK_VARIANT_TYPE_UINT8:
        return QVariant(arg->value.uint8);
    case TWEAK_VARIANT_TYPE_UINT16:
        return QVariant(arg->value.uint16);
    case TWEAK_VARIANT_TYPE_UINT32:
        return QVariant(arg->value.uint32);
    case TWEAK_VARIANT_TYPE_UINT64:
        return QVariant(static_cast<qulonglong>(arg->value.uint64));
    case TWEAK_VARIANT_TYPE_FLOAT:
        return QVariant(arg->value.fp32);
    case TWEAK_VARIANT_TYPE_DOUBLE:
        return QVariant(arg->value.fp64);
    default:
        Q_UNREACHABLE();
        break;
    }
}

static inline void to_tweak_variant(tweak_variant *dest, tweak_variant_type dest_type, const QVariant &src)
{
    dest->type = dest_type;
    switch (dest_type)
    {
    case TWEAK_VARIANT_TYPE_NULL:
        break;
    case TWEAK_VARIANT_TYPE_BOOL:
        dest->value.b = static_cast<bool>(src.toBool());
        break;
    case TWEAK_VARIANT_TYPE_SINT8:
        dest->value.sint8 = static_cast<int8_t>(src.toInt());
        break;
    case TWEAK_VARIANT_TYPE_SINT16:
        dest->value.sint16 = static_cast<int16_t>(src.toInt());
        break;
    case TWEAK_VARIANT_TYPE_SINT32:
        dest->value.sint32 = static_cast<int32_t>(src.toInt());
        break;
    case TWEAK_VARIANT_TYPE_SINT64:
        dest->value.sint64 = static_cast<int64_t>(src.toLongLong());
        break;
    case TWEAK_VARIANT_TYPE_UINT8:
        dest->value.uint8 = static_cast<uint8_t>(src.toUInt());
        break;
    case TWEAK_VARIANT_TYPE_UINT16:
        dest->value.uint16 = static_cast<uint16_t>(src.toUInt());
        break;
    case TWEAK_VARIANT_TYPE_UINT32:
        dest->value.uint32 = static_cast<uint32_t>(src.toUInt());
        break;
    case TWEAK_VARIANT_TYPE_UINT64:
        dest->value.uint64 = static_cast<uint64_t>(src.toULongLong());
        break;
    case TWEAK_VARIANT_TYPE_FLOAT:
        dest->value.fp32 = static_cast<float>(src.toFloat());
        break;
    case TWEAK_VARIANT_TYPE_DOUBLE:
        dest->value.fp64 = static_cast<double>(src.toDouble());
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

} // namespace tweak2

#endif // QTWEAKVARIANT_H
