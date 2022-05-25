/**
 * @file TweakMetadataParser.hpp
 * @ingroup GUI
 *
 * @brief Metdata parser for Tweak QML GUI.
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

#ifndef TWEAKMETADATAPARSER_HPP
#define TWEAKMETADATAPARSER_HPP

#include <tweak2/variant.h>

#include <QObject>
#include <QVariantMap>
#include <QAbstractListModel>

#include <tweak2/metadata.h>

namespace tweak2
{

class OptionsModel : public QAbstractListModel
{
    Q_OBJECT
    tweak_metadata_options m_options;
public:
    enum OptionsRoles {
        ValueRole = Qt::UserRole + 1,
        TextRole
    };

    OptionsModel(QObject *parent = nullptr);
    OptionsModel(tweak_metadata_options options, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE int find(QVariant value) const;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
};

/**
 * @brief Metadata information about individual tweak control that is used to select
 *        proper elements in the GUI.
 */
class TweakMetadata : public QObject
{
    Q_OBJECT
    tweak_metadata m_metadata;
    OptionsModel* m_options;
public:
    enum class ControlType {
        Unknown,
        Checkbox,
        Button,
        Spinbox,
        Slider,
        Combobox
    };
    Q_ENUM(ControlType);
    TweakMetadata(QObject *parent = nullptr);
    TweakMetadata(tweak_metadata metadata, QObject *parent = nullptr);
    ~TweakMetadata();

    ControlType getControlType() const;
    QVariant getMin() const;
    QVariant getMax() const;
    bool getReadonly() const;
    quint32 getDecimals() const;
    QVariant getStep() const;
    QString getCaption() const;
    OptionsModel* getOptions() const;

    Q_PROPERTY(ControlType controlType READ getControlType CONSTANT)
    Q_PROPERTY(QVariant min READ getMin CONSTANT)
    Q_PROPERTY(QVariant max READ getMax CONSTANT)
    Q_PROPERTY(bool readonly READ getReadonly CONSTANT)
    Q_PROPERTY(QVariant step READ getStep CONSTANT)
    Q_PROPERTY(quint32 decimals READ getDecimals CONSTANT)
    Q_PROPERTY(QString caption READ getCaption CONSTANT)
    Q_PROPERTY(OptionsModel* options READ getOptions CONSTANT)
};

class TweakMetadataParser : public QObject
{
    Q_OBJECT
public slots:
    /**
     * @brief Parser metadata and return standard structure.
     * @param meta
     * @return
     */
    TweakMetadata* parse(tweak_variant_type item_type, QString meta) const;
public:
    explicit TweakMetadataParser(QObject *parent = Q_NULLPTR);

};

}

Q_DECLARE_METATYPE(tweak2::TweakMetadata*)

#endif // TWEAKMETADATAPARSER_HPP
