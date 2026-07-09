#pragma once

#include "appsettings.h"
#include "openai_client.h"
#include "solutiondraft.h"

#include <QMainWindow>

class QCloseEvent;
class QLabel;
class QPushButton;
class QTextBrowser;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onGenerate();
    void onStop();
    void onCopy();
    void onExportHtml();
    void onOpenSettings();
    void onGenerationStarted();
    void onGenerationFinished(const SolutionDraft& draft, const QString& html);
    void onGenerationFailed(const QString& message);
    void onGenerationCancelled();

private:
    void setupUi();
    void loadState();
    void saveState();
    void refreshButtons();
    void setStatus(const QString& message, bool error = false);

    AppSettings m_settings;
    SolutionDraft m_currentDraft;
    QString m_currentHtml;
    bool m_busy = false;

    QTextEdit* m_inputEdit = nullptr;
    QTextBrowser* m_previewBrowser = nullptr;
    QLabel* m_statusLabel = nullptr;
    QPushButton* m_generateButton = nullptr;
    QPushButton* m_stopButton = nullptr;
    QPushButton* m_copyButton = nullptr;
    QPushButton* m_exportButton = nullptr;
    QPushButton* m_settingsButton = nullptr;

    OpenAIClient m_client;
};
