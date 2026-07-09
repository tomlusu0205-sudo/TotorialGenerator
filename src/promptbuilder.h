#pragma once

#include <QString>

namespace PromptBuilder {

QString systemPrompt();
QString userPrompt(const QString& problemDescription);

}

