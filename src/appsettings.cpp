#include "appsettings.h"

#include <QByteArray>
#include <QSettings>

QString AppSettings::resolvedApiKey() const
{
    const QString envKey = qEnvironmentVariable("OPENAI_API_KEY").trimmed();
    if (!apiKey.trimmed().isEmpty()) {
        return apiKey.trimmed();
    }
    return envKey;
}

AppSettings loadAppSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("openai"));

    AppSettings value;
    value.apiKey = settings.value(QStringLiteral("apiKey")).toString();
    value.apiBaseUrl = settings.value(QStringLiteral("apiBaseUrl"), value.apiBaseUrl).toString();
    value.model = settings.value(QStringLiteral("model"), value.model).toString();
    value.temperature = settings.value(QStringLiteral("temperature"), value.temperature).toDouble();
    value.maxTokens = settings.value(QStringLiteral("maxTokens"), value.maxTokens).toInt();
    value.requestTimeoutMs = settings.value(QStringLiteral("requestTimeoutMs"), value.requestTimeoutMs).toInt();

    settings.endGroup();
    return value;
}

void saveAppSettings(const AppSettings& settings)
{
    QSettings writer;
    writer.beginGroup(QStringLiteral("openai"));
    writer.setValue(QStringLiteral("apiKey"), settings.apiKey);
    writer.setValue(QStringLiteral("apiBaseUrl"), settings.apiBaseUrl);
    writer.setValue(QStringLiteral("model"), settings.model);
    writer.setValue(QStringLiteral("temperature"), settings.temperature);
    writer.setValue(QStringLiteral("maxTokens"), settings.maxTokens);
    writer.setValue(QStringLiteral("requestTimeoutMs"), settings.requestTimeoutMs);
    writer.endGroup();
}

void saveWindowGeometry(const QByteArray& geometry)
{
    QSettings writer;
    writer.setValue(QStringLiteral("window/geometry"), geometry);
}

QByteArray loadWindowGeometry()
{
    QSettings reader;
    return reader.value(QStringLiteral("window/geometry")).toByteArray();
}

