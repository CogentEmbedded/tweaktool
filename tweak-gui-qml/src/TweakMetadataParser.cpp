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

#include <QJsonDocument>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>


namespace tweak2
{

QStringList extractStringArray(const QJsonArray& jsonArray) {
    QStringList result;
    for (const QJsonValue &value : jsonArray) {
        result.push_back(value.toString());
    }
    return result;
}

TweakMetadata *readJsonMeta(const TweakMetadataParser *parser,
                            const QJsonObject &json,
                            const QString meta)
{
    Q_UNUSED(parser);
    TweakMetadata *m = new TweakMetadata();

    if (json.contains("readonly") && json["readonly"].isBool())
        m->m_readonly = json["readonly"].toBool();

    if (json.contains("min") && json["min"].isDouble())
        m->m_min = json["min"].toDouble();

    if (json.contains("max") && json["max"].isDouble())
        m->m_max = json["max"].toDouble();

    if (json.contains("step") && json["step"].isDouble())
        m->m_max = json["step"].toDouble();

    QString controlType;
    if (json.contains("type") && json["type"].isString()) {
        controlType = json["type"].toString();
    }

    if (json.contains("options") && json["options"].isArray()) {
        controlType = "enum";
        m->m_options = extractStringArray(json["options"].toArray());
    }

    m->m_controlType = TweakMetadata::ControlType::GENERIC;
    if (controlType.contains("int"))
    {
        m->m_decimals = 0;
    }
    else if (controlType.contains("bool"))
    {
        m->m_decimals = 0;
        m->m_controlType = TweakMetadata::ControlType::TOGGLE;
    }
    else if (controlType.contains("enum"))
    {
        m->m_decimals = 0;
        m->m_controlType = TweakMetadata::ControlType::ENUM;
    }

    m->m_raw = meta;

    return m;
}

TweakMetadata *TweakMetadataParser::parse(const QString meta) const
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(meta.toLocal8Bit(), &error);

    return readJsonMeta(this, doc.object(), meta);
}

TweakMetadataParser::TweakMetadataParser(QObject *parent)
    : QObject(parent)
{

}


}
