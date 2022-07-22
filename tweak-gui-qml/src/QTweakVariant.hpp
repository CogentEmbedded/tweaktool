/**
 * @file QTweakVariant.hpp
 * @ingroup GUI
 *
 * @brief Qt binding for Tweak Variant.
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

#ifndef QTWEAKVARIANT_H
#define QTWEAKVARIANT_H

#include <QString>
#include <QVariant>
#include <QByteArray>
#include <QJsonDocument>

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
    tweak_assign_string(dest, src0.constData());
}

static inline QVariant to_json_node(const tweak_variant *arg)
{
    tweak_variant_string tmp = tweak_variant_to_string(arg);
    QByteArray documentSource(tweak_variant_string_c_str(&tmp));
    tweak_variant_destroy_string(&tmp);
    return QVariant::fromValue(QJsonDocument::fromJson(documentSource));
}

static inline tweak_variant from_json_node(const QVariant& arg, tweak_variant_type dest_type)
{
    (void)arg;
    (void)dest_type;
    Q_ASSERT(false); // Not implemented
    return TWEAK_VARIANT_INIT_EMPTY;
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
    case TWEAK_VARIANT_TYPE_STRING:
        return QVariant(QString(tweak_variant_string_c_str(&arg->value.string)));
    case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
        return to_json_node(arg);
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
    case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
        *dest = from_json_node(src, dest_type);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

} // namespace tweak2

#endif // QTWEAKVARIANT_H
