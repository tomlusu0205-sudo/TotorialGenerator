#pragma once

#include <QByteArray>
#include <QString>

struct AppSettings
{
    QString apiKey;
    QString apiBaseUrl = QStringLiteral("https://api.openai.com/v1");
    QString model = QStringLiteral("gpt-4.1-mini");
    double temperature = 0.2;
    int maxTokens = 2200;
    int requestTimeoutMs = 120000;

    QString resolvedApiKey() const;
};

AppSettings loadAppSettings();
void saveAppSettings(const AppSettings& settings);
void saveWindowGeometry(const QByteArray& geometry);
QByteArray loadWindowGeometry();
