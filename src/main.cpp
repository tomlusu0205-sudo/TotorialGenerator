#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("TomCodex"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("tom-codex.local"));
    QCoreApplication::setApplicationName(QStringLiteral("ProblemSolutionGenerator"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    MainWindow window;
    window.show();

    return app.exec();
}

