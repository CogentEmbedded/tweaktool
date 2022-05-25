/**
 * @file TweakMetadata.test.cpp
 * @ingroup GUI
 *
 * @brief test suite for Qt binding for tweak metadata.
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

#include "TweakMetadataParser.hpp"

#include <limits>
#include <cmath>

/**
 * @details Tests tweak_variant to QVariant conversions.
 */
namespace tweak2
{

namespace { }

class QMetadataTest : public QObject
{
   Q_OBJECT
   private slots:
   void Test1() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_BOOL, nullptr);
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Checkbox);
      QVERIFY(metadata->getMin() == false);
      QVERIFY(metadata->getMax() == true);
      QVERIFY(metadata->getReadonly() == false);
      delete metadata;
      delete parser;
   }

   void Test2() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_BOOL, "{ \"readonly\": true }");
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Checkbox);
      QVERIFY(metadata->getMin() == false);
      QVERIFY(metadata->getMax() == true);
      QVERIFY(metadata->getReadonly() == true);
      delete metadata;
      delete parser;
   }


   static QString getOptionText(TweakMetadata* metadata, int row) {
        OptionsModel* optionsModel = metadata->getOptions();
        if (optionsModel != nullptr) {
            return optionsModel->data(optionsModel->index(row), OptionsModel::OptionsRoles::TextRole).toString();
        }
       return QString();
   }

   static QVariant getOptionValue(TweakMetadata* metadata, int row) {
        OptionsModel* optionsModel = metadata->getOptions();
        if (optionsModel != nullptr) {
            return optionsModel->data(optionsModel->index(row), OptionsModel::OptionsRoles::ValueRole);
        }
       return QVariant();
   }

   void Test4() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_BOOL, "{ \"options\": [\"True\", \"False\"] }");
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Combobox);
      QVERIFY(getOptionText(metadata, 0) == "True");
      QVERIFY(getOptionText(metadata, 1) == "False");
      QVERIFY(metadata->getReadonly() == false);
      delete metadata;
      delete parser;
   }

   void Test5() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_BOOL, "{ \"options\": [\"True\", \"False\"] }");
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Combobox);
      QVERIFY(getOptionText(metadata, 0) == "True");
      QVERIFY(getOptionText(metadata, 1) == "False");
      QVERIFY(metadata->getReadonly() == false);
      delete metadata;
      delete parser;
   }

   void Test6() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_FLOAT, nullptr);
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Slider);
      QVERIFY(metadata->getMin() == -std::numeric_limits<float>::max());
      QVERIFY(metadata->getMax() == std::numeric_limits<float>::max());
      uint32_t decimals = metadata->getDecimals();
      QVERIFY(decimals == 4);
      QVERIFY(metadata->getReadonly() == false);
      float step = metadata->getStep().value<float>();
      float err = std::abs(step - .0001f); /* derived from decimals == 4*/
      QVERIFY(err < pow(10, -(decimals + 2)));
      delete metadata;
      delete parser;
   }

   void Test7() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_FLOAT, "{ \"min\": -1, \"max\": 1, \"decimals\": 6 }");
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Slider);
      QVERIFY(metadata->getMin() == -1.f);
      QVERIFY(metadata->getMax() == 1.f);
      uint32_t decimals = metadata->getDecimals();
      QVERIFY(decimals == 6);
      QVERIFY(metadata->getReadonly() == false);
      delete metadata;
      delete parser;
   }

   void Test8() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_UINT32, "{ \"min\": 0, \"max\": 512 }");
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Spinbox);
      QVERIFY(metadata->getMin().value<uint32_t>() == 0L);
      QVERIFY(metadata->getMax().value<uint32_t>() == 512L);
      QVERIFY(metadata->getReadonly() == false);
      delete metadata;
      delete parser;
   }

   void Test9() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_SINT32, "{ \"min\": -128, \"max\": 127 }");
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Spinbox);
      QVERIFY(metadata->getMin().value<int32_t>() == -128);
      QVERIFY(metadata->getMax().value<int32_t>() == 127);
      QVERIFY(metadata->getReadonly() == false);
      delete metadata;
      delete parser;
   }

   void Test13() {
      TweakMetadataParser* parser = new TweakMetadataParser();
      TweakMetadata* metadata = parser->parse(TWEAK_VARIANT_TYPE_UINT32,
         "{\"readonly\":false, \"type\":\"uint64_t\", \"min\": 0.00000, \"decimals\": 2.1, \"max\": 4.00000}"
      );
      QVERIFY(metadata->getControlType() == tweak2::TweakMetadata::ControlType::Spinbox);
      QVERIFY(metadata->getMin().value<uint32_t>() == 0);
      uint64_t v = metadata->getMax().value<uint64_t>();
      QVERIFY(v == 4);
      QVERIFY(metadata->getDecimals() == 2);
      QVERIFY(metadata->getReadonly() == false);
      delete metadata;
      delete parser;
   }
};

} // namespace tweak2

#include "TweakMetadata.test.moc"
QTEST_MAIN(tweak2::QMetadataTest)
