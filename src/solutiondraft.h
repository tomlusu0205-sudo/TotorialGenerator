#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct ExampleCase
{
    QString input;
    QString output;
    QString explanation;
};

struct SolutionDraft
{
    QString title;
    QString problemType;
    QString difficulty;
    QString overview;
    QString algorithmAnalysis;
    QString timeComplexity;
    QString spaceComplexity;
    QStringList cppFramework;
    QVector<ExampleCase> examples;
    QStringList commonMistakes;
    QStringList assumptions;

    bool isEmpty() const;
    QString toPlainText() const;
};

