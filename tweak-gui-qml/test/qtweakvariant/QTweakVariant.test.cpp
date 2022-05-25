/**
 * @file QTweakVariant.test.cpp
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

#include "float.h"
#include "QTweakVariant.hpp"


/**
 * @details Tests tweak_variant to QVariant conversions.
 */
namespace tweak2
{

namespace {
QVariant generateRandomQtVariantValue(QMetaType::Type metaType) {
    QVariant result;
    switch (static_cast<QMetaType::Type>(metaType)) {
    case QMetaType::Nullptr:
        result = QVariant::fromValue<std::nullptr_t>(nullptr);
        break;
    case QMetaType::Bool:
        result = QVariant::fromValue<bool>((rand() % 2) != 0);
        break;
    case QMetaType::Int:
        result = QVariant::fromValue<int>(static_cast<int>(rand()));
        break;
    case QMetaType::UInt:
        result = QVariant::fromValue<unsigned int>(static_cast<unsigned int>(rand()));
        break;
    case QMetaType::Long:
        result = QVariant::fromValue<long>(static_cast<long>(rand()));
        break;
    case QMetaType::LongLong:
        result = QVariant::fromValue<long long>(static_cast<long long>(rand()));
        break;
    case QMetaType::Short:
        result = QVariant::fromValue<short>(static_cast<short>(rand()));
        break;
    case QMetaType::UShort:
        result = QVariant::fromValue<unsigned short>(static_cast<unsigned short>(rand()));
        break;
    case QMetaType::ULong:
        result = QVariant::fromValue<unsigned long>(static_cast<unsigned long>(rand()));
        break;
    case QMetaType::ULongLong:
        result = QVariant::fromValue<unsigned long long>(static_cast<unsigned long long>(rand()));
        break;
    case QMetaType::Float:
        result = QVariant::fromValue<float>((float)rand() / (float)RAND_MAX * 2000.f - 1000.f);
        break;
    case QMetaType::Double:
        result = QVariant::fromValue<double>((double)rand() / (double)RAND_MAX * 2000. - 1000.);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    return result;
}

template<typename T> struct CheckConv {
    static void checkValue(T src, tweak_variant* dest);
};

template<typename T>
    void CheckConv<T>::checkValue(T src, tweak_variant* dest)
{
    bool checkResult;
    QVariant check = QVariant::fromValue(src);
    switch (dest->type) {
    case TWEAK_VARIANT_TYPE_NULL:
        checkResult = true;
        break;
    case TWEAK_VARIANT_TYPE_BOOL:
        checkResult = check.toBool() == dest->value.b;
        break;
    case TWEAK_VARIANT_TYPE_SINT8:
        checkResult = static_cast<int8_t>(check.toInt()) == dest->value.sint8;
        break;
    case TWEAK_VARIANT_TYPE_SINT16:
        checkResult = static_cast<int16_t>(check.toInt()) ==  dest->value.sint16;
        break;
    case TWEAK_VARIANT_TYPE_SINT32:
        checkResult = static_cast<int32_t>(check.toInt()) ==  dest->value.sint32;
        break;
    case TWEAK_VARIANT_TYPE_SINT64:
        checkResult = static_cast<int64_t>(check.toLongLong()) ==  dest->value.sint64;
        break;
    case TWEAK_VARIANT_TYPE_UINT8:
        checkResult = static_cast<uint8_t>(check.toUInt()) == dest->value.uint8;
        break;
    case TWEAK_VARIANT_TYPE_UINT16:
        checkResult = static_cast<uint16_t>(check.toUInt()) == dest->value.uint16;
        break;
    case TWEAK_VARIANT_TYPE_UINT32:
        checkResult = static_cast<uint32_t>(check.toUInt()) == dest->value.uint32;
        break;
    case TWEAK_VARIANT_TYPE_UINT64:
        checkResult = static_cast<uint64_t>(check.toULongLong()) == dest->value.uint64;
        break;
    case TWEAK_VARIANT_TYPE_FLOAT:
        checkResult = check.toFloat() == dest->value.fp32;
        break;
    case TWEAK_VARIANT_TYPE_DOUBLE:
        checkResult = check.toDouble() == dest->value.fp64;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    QVERIFY(checkResult);
}

void checkConversionValidity(const QVariant &src, tweak_variant* dest) {
    switch (static_cast<QMetaType::Type>(src.type())) {
    case QMetaType::Nullptr:
        break;
    case QMetaType::Bool:
        CheckConv<bool>::checkValue(src.toBool(), dest);
        break;
    case QMetaType::Int:
        CheckConv<int>::checkValue(src.toInt(), dest);
        break;
    case QMetaType::UInt:
        CheckConv<unsigned int>::checkValue(src.toUInt(), dest);
        break;
    case QMetaType::Long:
        CheckConv<long>::checkValue(src.toInt(), dest);
        break;
    case QMetaType::LongLong:
        CheckConv<long long>::checkValue(src.toLongLong(), dest);
        break;
    case QMetaType::Short:
        CheckConv<short>::checkValue(src.toInt(), dest);
        break;
    case QMetaType::UShort:
        CheckConv<unsigned short>::checkValue(src.toUInt(), dest);
        break;
    case QMetaType::ULong:
        CheckConv<unsigned long>::checkValue(src.toUInt(), dest);
        break;
    case QMetaType::ULongLong:
        CheckConv<unsigned long long>::checkValue(src.toULongLong(), dest);
        break;
    case QMetaType::Float:
        CheckConv<float>::checkValue(src.toFloat(), dest);
        break;
    case QMetaType::Double:
        CheckConv<double>::checkValue(src.toDouble(), dest);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

}

class QTweakVariantTest : public QObject
{
    Q_OBJECT

    template <typename F, typename T>
    void testTweakVariantScalar(F f, T example = 0,
                                bool need_type_coercion = false)
    {
        (void)need_type_coercion;
        tweak_variant tv = TWEAK_VARIANT_INIT_EMPTY;
        (*f)(&tv, example);
        QVERIFY(tv.type != TWEAK_VARIANT_TYPE_NULL);

        QVariant qv = from_tweak_variant(&tv);
        QVERIFY(qv.isValid());
    }

  private slots:
    void Scalar()
    {
        testTweakVariantScalar(&tweak_variant_assign_bool, true);

        testTweakVariantScalar(&tweak_variant_assign_uint8,
                               static_cast<uint8_t>(UINT8_MAX), true);
        testTweakVariantScalar(&tweak_variant_assign_sint8,
                               static_cast<int8_t>(INT8_MAX), true);

        testTweakVariantScalar(&tweak_variant_assign_uint16,
                               static_cast<uint16_t>(UINT16_MAX), true);
        testTweakVariantScalar(&tweak_variant_assign_sint16,
                               static_cast<int16_t>(INT16_MAX), true);

        testTweakVariantScalar(&tweak_variant_assign_uint32, UINT32_MAX);
        testTweakVariantScalar(&tweak_variant_assign_sint32, INT32_MAX);

        testTweakVariantScalar(&tweak_variant_assign_uint64, UINT64_MAX);
        testTweakVariantScalar(&tweak_variant_assign_sint64, INT64_MAX);

        testTweakVariantScalar(&tweak_variant_assign_float, FLT_MAX);
        testTweakVariantScalar(&tweak_variant_assign_double, DBL_MAX);
    }

    void TestTo()
    {
        std::vector<QMetaType::Type> metaVariantTypes {
            QMetaType::Nullptr,
            QMetaType::Bool,
            QMetaType::Int,
            QMetaType::UInt,
            QMetaType::Long,
            QMetaType::LongLong,
            QMetaType::Short,
            QMetaType::UShort,
            QMetaType::ULong,
            QMetaType::ULongLong,
            QMetaType::Float,
            QMetaType::Double,
        };

        std::vector<tweak_variant_type> tweakVariantTypes {
            TWEAK_VARIANT_TYPE_NULL,
            TWEAK_VARIANT_TYPE_BOOL,
            TWEAK_VARIANT_TYPE_SINT8,
            TWEAK_VARIANT_TYPE_SINT16,
            TWEAK_VARIANT_TYPE_SINT32,
            TWEAK_VARIANT_TYPE_SINT64,
            TWEAK_VARIANT_TYPE_UINT8,
            TWEAK_VARIANT_TYPE_UINT16,
            TWEAK_VARIANT_TYPE_UINT32,
            TWEAK_VARIANT_TYPE_UINT64,
            TWEAK_VARIANT_TYPE_FLOAT,
            TWEAK_VARIANT_TYPE_DOUBLE
        };

        for (QMetaType::Type metaType : metaVariantTypes) {
            QVariant randomValue = generateRandomQtVariantValue(metaType);
            for (tweak_variant_type tweakVariantType : tweakVariantTypes) {
                tweak_variant value = {};
                to_tweak_variant(&value, tweakVariantType, randomValue);
                QVERIFY(value.type == tweakVariantType);
                checkConversionValidity(randomValue, &value);
            }
        }
    }
};

} // namespace tweak2

#include "QTweakVariant.test.moc"
QTEST_MAIN(tweak2::QTweakVariantTest)
