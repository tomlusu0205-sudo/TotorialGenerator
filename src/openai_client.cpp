#include "openai_client.h"

#include "htmlrenderer.h"
#include "promptbuilder.h"
#include "responseparser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

QString buildUrl(const QString& baseUrl, const QString& path)
{
    QUrl url(baseUrl.trimmed());
    QString cleanPath = url.path();
    if (!cleanPath.endsWith(QLatin1Char('/'))) {
        cleanPath += QLatin1Char('/');
    }
    cleanPath += path.startsWith(QLatin1Char('/')) ? path.mid(1) : path;
    url.setPath(cleanPath);
    return url.toString(QUrl::FullyEncoded);
}

OpenAIClient::OpenAIClient(QObject* parent)
    : QObject(parent)
{
    m_timeoutTimer.setSingleShot(true);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &OpenAIClient::handleTimeout);
}

void OpenAIClient::generate(const QString& problemDescription, const AppSettings& settings)
{
    if (isBusy()) {
        cancel();
    }

    const QString apiKey = settings.resolvedApiKey();
    if (apiKey.trimmed().isEmpty()) {
        emit failed(QStringLiteral("没有可用的 OpenAI API Key，请先在设置里填写，或设置环境变量 OPENAI_API_KEY。"));
        return;
    }

    const QString description = problemDescription.trimmed();
    if (description.isEmpty()) {
        emit failed(QStringLiteral("题目描述不能为空。"));
        return;
    }

    QUrl url(buildUrl(settings.apiBaseUrl.isEmpty() ? QStringLiteral("https://api.openai.com/v1") : settings.apiBaseUrl,
                      QStringLiteral("/chat/completions")));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());
    request.setRawHeader("Accept", QByteArray("application/json"));

    QJsonObject body;
    body.insert(QStringLiteral("model"), settings.model);
    body.insert(QStringLiteral("temperature"), settings.temperature);
    body.insert(QStringLiteral("max_tokens"), settings.maxTokens);
    body.insert(QStringLiteral("response_format"), QJsonObject{{QStringLiteral("type"), QStringLiteral("json_object")}});

    QJsonArray messages;
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("system")},
        {QStringLiteral("content"), PromptBuilder::systemPrompt()}
    });
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("user")},
        {QStringLiteral("content"), PromptBuilder::userPrompt(description)}
    });
    body.insert(QStringLiteral("messages"), messages);

    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    m_reply = m_network.post(request, payload);
    m_timeoutTimer.start(settings.requestTimeoutMs);

    connect(m_reply, &QNetworkReply::finished, this, &OpenAIClient::handleReplyFinished);

    emit started();
}

void OpenAIClient::cancel()
{
    if (m_reply) {
        m_reply->abort();
    }
    if (m_timeoutTimer.isActive()) {
        m_timeoutTimer.stop();
    }
    cleanupReply();
    emit cancelled();
}

bool OpenAIClient::isBusy() const
{
    return !m_reply.isNull();
}

void OpenAIClient::cleanupReply()
{
    if (m_reply) {
        m_reply->deleteLater();
        m_reply.clear();
    }
}

void OpenAIClient::handleReplyFinished()
{
    if (!m_reply) {
        return;
    }

    const QByteArray replyBody = m_reply->readAll();
    const QNetworkReply::NetworkError error = m_reply->error();
    const QString networkErrorText = m_reply->errorString();

    m_timeoutTimer.stop();

    if (error == QNetworkReply::OperationCanceledError) {
        cleanupReply();
        emit cancelled();
        return;
    }

    if (error != QNetworkReply::NoError) {
        cleanupReply();
        emit failed(QStringLiteral("网络请求失败：%1").arg(networkErrorText));
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument rootDoc = QJsonDocument::fromJson(replyBody, &parseError);
    if (parseError.error != QJsonParseError::NoError || !rootDoc.isObject()) {
        cleanupReply();
        emit failed(QStringLiteral("OpenAI 响应不是有效 JSON：%1").arg(parseError.errorString()));
        return;
    }

    QString contentError;
    QString content = ResponseParser::extractAssistantContent(replyBody, &contentError);
    if (content.trimmed().isEmpty()) {
        cleanupReply();
        emit failed(QStringLiteral("OpenAI 返回内容为空：%1").arg(contentError));
        return;
    }

    QString draftError;
    const SolutionDraft draft = ResponseParser::parseSolutionDraftJson(content, &draftError);
    if (draft.isEmpty() && !draftError.isEmpty()) {
        cleanupReply();
        emit failed(QStringLiteral("题解解析失败：%1").arg(draftError));
        return;
    }

    const QString html = HtmlRenderer::render(draft);
    cleanupReply();
    emit finished(draft, html);
}

void OpenAIClient::handleTimeout()
{
    if (!m_reply) {
        return;
    }
    m_reply->abort();
    cleanupReply();
    emit failed(QStringLiteral("请求超时，请检查网络或适当提高超时时间。"));
}
