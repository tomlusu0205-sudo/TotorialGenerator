#include "responseparser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QString jsonErrorMessage(const QJsonObject& root)
{
    const QJsonValue errorValue = root.value(QStringLiteral("error"));
    if (errorValue.isObject()) {
        const QJsonObject errorObject = errorValue.toObject();
        const QString message = errorObject.value(QStringLiteral("message")).toString();
        const QString type = errorObject.value(QStringLiteral("type")).toString();
        if (!message.isEmpty() && !type.isEmpty()) {
            return type + QStringLiteral(": ") + message;
        }
        if (!message.isEmpty()) {
            return message;
        }
    }
    return {};
}

QString jsonTextValue(const QJsonValue& value)
{
    if (value.isString()) {
        return value.toString();
    }
    if (value.isDouble()) {
        return QString::number(value.toDouble());
    }
    if (value.isBool()) {
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    return {};
}

QString stripCodeFences(QString text)
{
    text = text.trimmed();
    if (text.startsWith(QStringLiteral("```"))) {
        const int firstNewline = text.indexOf(QLatin1Char('\n'));
        if (firstNewline >= 0) {
            text.remove(0, firstNewline + 1);
        }
        if (text.endsWith(QStringLiteral("```"))) {
            text.chop(3);
        }
    }
    return text.trimmed();
}

QStringList toStringList(const QJsonValue& value)
{
    QStringList result;
    if (value.isArray()) {
        const QJsonArray array = value.toArray();
        for (const QJsonValue& item : array) {
            const QString text = jsonTextValue(item).trimmed();
            if (!text.isEmpty()) {
                result.append(text);
            }
        }
    } else {
        const QString text = jsonTextValue(value).trimmed();
        if (!text.isEmpty()) {
            result.append(text);
        }
    }
    return result;
}

QVector<ExampleCase> parseExamples(const QJsonValue& value)
{
    QVector<ExampleCase> examples;
    if (!value.isArray()) {
        return examples;
    }

    const QJsonArray array = value.toArray();
    for (const QJsonValue& item : array) {
        if (!item.isObject()) {
            continue;
        }
        const QJsonObject obj = item.toObject();
        ExampleCase ex;
        ex.input = obj.value(QStringLiteral("input")).toString();
        ex.output = obj.value(QStringLiteral("output")).toString();
        ex.explanation = obj.value(QStringLiteral("explanation")).toString();
        if (!ex.input.trimmed().isEmpty() || !ex.output.trimmed().isEmpty() || !ex.explanation.trimmed().isEmpty()) {
            examples.append(ex);
        }
    }
    return examples;
}

} // namespace

namespace ResponseParser {

QString extractAssistantContent(const QByteArray& responseBody, QString* errorMessage)
{
    QJsonParseError parseError;
    const QJsonDocument rootDoc = QJsonDocument::fromJson(responseBody, &parseError);
    if (parseError.error != QJsonParseError::NoError || !rootDoc.isObject()) {
        if (errorMessage) {
            *errorMessage = parseError.errorString();
        }
        return {};
    }

    const QJsonObject root = rootDoc.object();
    const QString apiError = jsonErrorMessage(root);
    if (!apiError.isEmpty()) {
        if (errorMessage) {
            *errorMessage = apiError;
        }
        return {};
    }

    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("缺少 choices 字段。");
        }
        return {};
    }

    const QJsonObject choice = choices.first().toObject();
    const QJsonObject message = choice.value(QStringLiteral("message")).toObject();
    QString content = message.value(QStringLiteral("content")).toString().trimmed();
    if (content.isEmpty()) {
        content = choice.value(QStringLiteral("text")).toString().trimmed();
    }
    if (content.startsWith(QStringLiteral("```"))) {
        content = stripCodeFences(content);
    }
    if (content.isEmpty() && errorMessage) {
        *errorMessage = QStringLiteral("choices.message.content 为空。");
    }
    return content;
}

SolutionDraft parseSolutionDraftJson(const QString& jsonText, QString* errorMessage)
{
    const QString cleaned = stripCodeFences(jsonText);

    auto parseDocument = [](const QByteArray& bytes, QString* error) -> QJsonDocument {
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            if (error) {
                *error = parseError.errorString();
            }
            return {};
        }
        return doc;
    };

    QJsonDocument doc = parseDocument(cleaned.toUtf8(), errorMessage);
    if (doc.isNull()) {
        const int begin = cleaned.indexOf(QLatin1Char('{'));
        const int end = cleaned.lastIndexOf(QLatin1Char('}'));
        if (begin >= 0 && end > begin) {
            doc = parseDocument(cleaned.mid(begin, end - begin + 1).toUtf8(), errorMessage);
        }
    }

    if (doc.isNull()) {
        return {};
    }

    const QJsonObject root = doc.object();
    SolutionDraft draft;
    draft.title = root.value(QStringLiteral("title")).toString().trimmed();
    draft.problemType = root.value(QStringLiteral("problemType")).toString().trimmed();
    draft.difficulty = root.value(QStringLiteral("difficulty")).toString().trimmed();
    draft.overview = root.value(QStringLiteral("overview")).toString().trimmed();
    draft.algorithmAnalysis = root.value(QStringLiteral("algorithmAnalysis")).toString().trimmed();
    draft.timeComplexity = root.value(QStringLiteral("timeComplexity")).toString().trimmed();
    draft.spaceComplexity = root.value(QStringLiteral("spaceComplexity")).toString().trimmed();
    draft.cppFramework = toStringList(root.value(QStringLiteral("cppFramework")));
    draft.examples = parseExamples(root.value(QStringLiteral("examples")));
    draft.commonMistakes = toStringList(root.value(QStringLiteral("commonMistakes")));
    draft.assumptions = toStringList(root.value(QStringLiteral("assumptions")));

    if (draft.title.isEmpty()) {
        draft.title = QStringLiteral("自动生成题解");
    }
    if (draft.problemType.isEmpty()) {
        draft.problemType = QStringLiteral("其他");
    }
    if (draft.difficulty.isEmpty()) {
        draft.difficulty = QStringLiteral("未知");
    }

    return draft;
}

} // namespace ResponseParser

