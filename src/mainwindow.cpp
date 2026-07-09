#include "mainwindow.h"

#include "htmlrenderer.h"
#include "settingsdialog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

QString appStyleSheet()
{
    return QStringLiteral(R"(
QMainWindow {
    background: #f3f6fb;
}
QTextEdit, QTextBrowser {
    border: 1px solid #d8e0ea;
    border-radius: 12px;
    background: white;
    selection-background-color: #bfdbfe;
    font-family: "Segoe UI", "PingFang SC", "Microsoft YaHei", sans-serif;
    font-size: 14px;
    line-height: 1.5;
}
QPushButton {
    padding: 9px 16px;
    border-radius: 10px;
    border: 1px solid #cbd5e1;
    background: #ffffff;
}
QPushButton:hover {
    background: #eff6ff;
    border-color: #93c5fd;
}
QPushButton:disabled {
    color: #94a3b8;
    background: #f8fafc;
}
#PrimaryButton {
    background: #2563eb;
    color: white;
    border: 1px solid #1d4ed8;
    font-weight: 600;
}
#PrimaryButton:hover {
    background: #1d4ed8;
}
#DangerButton {
    background: #fee2e2;
    border: 1px solid #fca5a5;
    color: #991b1b;
}
#StatusLabel {
    color: #475569;
}
)");
}

QString escapeClipboardText(const QString& text)
{
    return text.trimmed().isEmpty() ? QStringLiteral("暂无生成结果。") : text;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("题目描述到题解生成器"));
    resize(1500, 920);

    setupUi();
    loadState();

    connect(&m_client, &OpenAIClient::started, this, &MainWindow::onGenerationStarted);
    connect(&m_client, &OpenAIClient::finished, this, &MainWindow::onGenerationFinished);
    connect(&m_client, &OpenAIClient::failed, this, &MainWindow::onGenerationFailed);
    connect(&m_client, &OpenAIClient::cancelled, this, &MainWindow::onGenerationCancelled);

    refreshButtons();
    setStatus(QStringLiteral("准备就绪。输入题目描述后点击“生成”。"));
}

MainWindow::~MainWindow()
{
    saveState();
}

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(12);

    auto* topBar = new QWidget(central);
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);

    m_settingsButton = new QPushButton(QStringLiteral("设置"), topBar);
    m_generateButton = new QPushButton(QStringLiteral("生成"), topBar);
    m_generateButton->setObjectName(QStringLiteral("PrimaryButton"));
    m_stopButton = new QPushButton(QStringLiteral("停止"), topBar);
    m_stopButton->setObjectName(QStringLiteral("DangerButton"));
    m_copyButton = new QPushButton(QStringLiteral("复制"), topBar);
    m_exportButton = new QPushButton(QStringLiteral("导出 HTML"), topBar);

    topLayout->addWidget(m_settingsButton);
    topLayout->addStretch(1);
    topLayout->addWidget(m_generateButton);
    topLayout->addWidget(m_stopButton);
    topLayout->addWidget(m_copyButton);
    topLayout->addWidget(m_exportButton);

    auto* splitter = new QSplitter(Qt::Horizontal, central);

    auto* leftPane = new QWidget(splitter);
    auto* leftLayout = new QVBoxLayout(leftPane);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);
    auto* inputLabel = new QLabel(QStringLiteral("题目描述"), leftPane);
    inputLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #0f172a;"));
    m_inputEdit = new QTextEdit(leftPane);
    m_inputEdit->setPlaceholderText(QStringLiteral("在这里粘贴题目描述，程序会调用 OpenAI API 自动生成结构化题解。"));
    leftLayout->addWidget(inputLabel);
    leftLayout->addWidget(m_inputEdit, 1);

    auto* rightPane = new QWidget(splitter);
    auto* rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);
    auto* previewLabel = new QLabel(QStringLiteral("生成结果预览"), rightPane);
    previewLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #0f172a;"));
    m_previewBrowser = new QTextBrowser(rightPane);
    m_previewBrowser->setOpenExternalLinks(true);
    m_previewBrowser->setHtml(QStringLiteral("<div style='color:#64748b;padding:16px;'>尚未生成内容。</div>"));
    rightLayout->addWidget(previewLabel);
    rightLayout->addWidget(m_previewBrowser, 1);

    splitter->addWidget(leftPane);
    splitter->addWidget(rightPane);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    m_statusLabel = new QLabel(central);
    m_statusLabel->setObjectName(QStringLiteral("StatusLabel"));
    m_statusLabel->setWordWrap(true);

    rootLayout->addWidget(topBar);
    rootLayout->addWidget(splitter, 1);
    rootLayout->addWidget(m_statusLabel);

    setCentralWidget(central);
    setStyleSheet(appStyleSheet());

    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::onGenerate);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(m_copyButton, &QPushButton::clicked, this, &MainWindow::onCopy);
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::onExportHtml);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onOpenSettings);
}

void MainWindow::loadState()
{
    m_settings = loadAppSettings();
    const QByteArray geometry = loadWindowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void MainWindow::saveState()
{
    saveAppSettings(m_settings);
    saveWindowGeometry(saveGeometry());
}

void MainWindow::refreshButtons()
{
    const bool hasDraft = !m_currentHtml.trimmed().isEmpty();
    m_generateButton->setEnabled(!m_busy);
    m_stopButton->setEnabled(m_busy);
    m_copyButton->setEnabled(hasDraft && !m_busy);
    m_exportButton->setEnabled(hasDraft && !m_busy);
    m_settingsButton->setEnabled(!m_busy);
}

void MainWindow::setStatus(const QString& message, bool error)
{
    m_statusLabel->setText(error ? QStringLiteral("<span style='color:#b91c1c;'>%1</span>").arg(message.toHtmlEscaped())
                                 : message.toHtmlEscaped());
}

void MainWindow::onGenerate()
{
    if (m_busy) {
        return;
    }

    const QString description = m_inputEdit->toPlainText().trimmed();
    if (description.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("题目描述不能为空。"));
        return;
    }

    if (m_settings.resolvedApiKey().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先在“设置”中填写 OpenAI API Key，或设置环境变量 OPENAI_API_KEY。"));
        return;
    }

    m_currentDraft = {};
    m_currentHtml.clear();
    m_previewBrowser->setHtml(QStringLiteral("<div style='color:#64748b;padding:16px;'>正在生成题解，请稍候...</div>"));
    setStatus(QStringLiteral("正在调用 OpenAI API 生成题解..."));
    refreshButtons();

    m_client.generate(description, m_settings);
}

void MainWindow::onStop()
{
    if (!m_busy) {
        return;
    }
    m_client.cancel();
}

void MainWindow::onCopy()
{
    if (m_currentDraft.isEmpty()) {
        return;
    }
    QApplication::clipboard()->setText(escapeClipboardText(m_currentDraft.toPlainText()));
    setStatus(QStringLiteral("已复制题解文本到剪贴板。"));
}

void MainWindow::onExportHtml()
{
    if (m_currentHtml.trimmed().isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("当前没有可导出的 HTML 内容。"));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("导出 HTML"),
        QStringLiteral("solution.html"),
        QStringLiteral("HTML 文件 (*.html);;所有文件 (*.*)"));

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::critical(this, QStringLiteral("导出失败"), QStringLiteral("无法写入文件：%1").arg(file.errorString()));
        return;
    }

    file.write(m_currentHtml.toUtf8());
    file.close();
    setStatus(QStringLiteral("HTML 已导出到 %1").arg(fileName));
}

void MainWindow::onOpenSettings()
{
    SettingsDialog dialog(this);
    dialog.setSettings(m_settings);
    if (dialog.exec() == QDialog::Accepted) {
        m_settings = dialog.settings();
        saveAppSettings(m_settings);
        setStatus(QStringLiteral("设置已保存。"));
    }
}

void MainWindow::onGenerationStarted()
{
    m_busy = true;
    refreshButtons();
}

void MainWindow::onGenerationFinished(const SolutionDraft& draft, const QString& html)
{
    m_busy = false;
    m_currentDraft = draft;
    m_currentHtml = html;
    m_previewBrowser->setHtml(html);
    setStatus(QStringLiteral("题解生成完成，可以复制或导出 HTML。"));
    refreshButtons();
}

void MainWindow::onGenerationFailed(const QString& message)
{
    m_busy = false;
    refreshButtons();
    m_previewBrowser->setHtml(QStringLiteral("<div style='color:#b91c1c;padding:16px;'>生成失败：%1</div>").arg(message.toHtmlEscaped()));
    setStatus(message, true);
    QMessageBox::critical(this, QStringLiteral("生成失败"), message);
}

void MainWindow::onGenerationCancelled()
{
    m_busy = false;
    refreshButtons();
    setStatus(QStringLiteral("生成已取消。"));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveState();
    QMainWindow::closeEvent(event);
}
