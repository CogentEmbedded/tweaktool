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

#ifndef TWEAKMETADATAPARSER_HPP
#define TWEAKMETADATAPARSER_HPP

#include <QObject>

namespace tweak2
{

/**
 * @brief Metadata information about individual tweak control that is used to select
 *        proper elements in the GUI.
 */
class TweakMetadata : public QObject
{
    Q_OBJECT

public:
    QString m_tweakType;
    bool m_readonly = false;

    double m_min = 0;
    double m_max = 1;
    double m_step = 1;

    QString m_raw = "";

    quint32 m_decimals = 3;

    bool m_toggle = false;

public:
    Q_PROPERTY(QString tweakType MEMBER m_tweakType CONSTANT)
    Q_PROPERTY(bool readonly MEMBER m_readonly CONSTANT)
    Q_PROPERTY(double min MEMBER m_min CONSTANT)
    Q_PROPERTY(double max MEMBER m_max CONSTANT)
    Q_PROPERTY(double step MEMBER m_step CONSTANT)
    Q_PROPERTY(QString raw MEMBER m_raw CONSTANT)
    Q_PROPERTY(quint32 decimals MEMBER m_decimals CONSTANT)
    Q_PROPERTY(bool toggle MEMBER m_toggle CONSTANT)
};


/**
 * @brief Parses tweak metadata in various formats.
 *
 * Supported formats include:
 *  - JSON
 *
 * Other formats can be added on request.
 */
class TweakMetadataParser : public QObject
{
    Q_OBJECT

private:


public slots:
    /**
     * @brief Parser metadata and return standard structure.
     * @param meta
     * @return
     */
    TweakMetadata* parse(const QString meta) const;


public:
    explicit TweakMetadataParser(QObject *parent = Q_NULLPTR);

};

}

Q_DECLARE_METATYPE(tweak2::TweakMetadata*);

#endif // TWEAKMETADATAPARSER_HPP
