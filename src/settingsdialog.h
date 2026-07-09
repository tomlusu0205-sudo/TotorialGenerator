#pragma once

#include "appsettings.h"

#include <QDialog>

class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void setSettings(const AppSettings& settings);
    AppSettings settings() const;

private:
    QLineEdit* m_apiKeyEdit = nullptr;
    QLineEdit* m_apiBaseUrlEdit = nullptr;
    QLineEdit* m_modelEdit = nullptr;
    QDoubleSpinBox* m_temperatureSpin = nullptr;
    QSpinBox* m_maxTokensSpin = nullptr;
    QSpinBox* m_timeoutSpin = nullptr;
};

