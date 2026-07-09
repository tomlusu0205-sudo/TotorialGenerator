#pragma once

#include "solutiondraft.h"

#include <QByteArray>
#include <QString>

namespace ResponseParser {

QString extractAssistantContent(const QByteArray& responseBody, QString* errorMessage = nullptr);
SolutionDraft parseSolutionDraftJson(const QString& jsonText, QString* errorMessage = nullptr);

}

