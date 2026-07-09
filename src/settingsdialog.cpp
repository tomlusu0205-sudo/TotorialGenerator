#include "settingsdialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("模型设置"));
    setModal(true);
    resize(540, 320);

    auto* mainLayout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    m_apiKeyEdit = new QLineEdit(this);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText(QStringLiteral("可留空，程序会回退到环境变量 OPENAI_API_KEY"));

    m_apiBaseUrlEdit = new QLineEdit(this);
    m_apiBaseUrlEdit->setPlaceholderText(QStringLiteral("https://api.openai.com/v1"));

    m_modelEdit = new QLineEdit(this);
    m_modelEdit->setPlaceholderText(QStringLiteral("gpt-4.1-mini"));

    m_temperatureSpin = new QDoubleSpinBox(this);
    m_temperatureSpin->setRange(0.0, 2.0);
    m_temperatureSpin->setSingleStep(0.1);
    m_temperatureSpin->setDecimals(2);

    m_maxTokensSpin = new QSpinBox(this);
    m_maxTokensSpin->setRange(128, 20000);
    m_maxTokensSpin->setSingleStep(128);

    m_timeoutSpin = new QSpinBox(this);
    m_timeoutSpin->setRange(15000, 600000);
    m_timeoutSpin->setSingleStep(5000);
    m_timeoutSpin->setSuffix(QStringLiteral(" ms"));

    form->addRow(QStringLiteral("API Key"), m_apiKeyEdit);
    form->addRow(QStringLiteral("API Base URL"), m_apiBaseUrlEdit);
    form->addRow(QStringLiteral("模型名称"), m_modelEdit);
    form->addRow(QStringLiteral("Temperature"), m_temperatureSpin);
    form->addRow(QStringLiteral("Max Tokens"), m_maxTokensSpin);
    form->addRow(QStringLiteral("请求超时"), m_timeoutSpin);

    mainLayout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

void SettingsDialog::setSettings(const AppSettings& settings)
{
    m_apiKeyEdit->setText(settings.apiKey);
    m_apiBaseUrlEdit->setText(settings.apiBaseUrl);
    m_modelEdit->setText(settings.model);
    m_temperatureSpin->setValue(settings.temperature);
    m_maxTokensSpin->setValue(settings.maxTokens);
    m_timeoutSpin->setValue(settings.requestTimeoutMs);
}

AppSettings SettingsDialog::settings() const
{
    AppSettings settings;
    settings.apiKey = m_apiKeyEdit->text().trimmed();
    settings.apiBaseUrl = m_apiBaseUrlEdit->text().trimmed();
    settings.model = m_modelEdit->text().trimmed();
    settings.temperature = m_temperatureSpin->value();
    settings.maxTokens = m_maxTokensSpin->value();
    settings.requestTimeoutMs = m_timeoutSpin->value();
    return settings;
}

