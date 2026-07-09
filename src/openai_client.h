#pragma once

#include "appsettings.h"
#include "solutiondraft.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QTimer>

class QNetworkReply;

class OpenAIClient : public QObject
{
    Q_OBJECT

public:
    explicit OpenAIClient(QObject* parent = nullptr);

    void generate(const QString& problemDescription, const AppSettings& settings);
    void cancel();
    bool isBusy() const;

signals:
    void started();
    void finished(const SolutionDraft& draft, const QString& html);
    void failed(const QString& message);
    void cancelled();

private:
    void cleanupReply();
    void handleReplyFinished();
    void handleTimeout();

    QNetworkAccessManager m_network;
    QPointer<QNetworkReply> m_reply;
    QTimer m_timeoutTimer;
};
