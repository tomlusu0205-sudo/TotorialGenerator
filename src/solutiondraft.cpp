#include "solutiondraft.h"

#include <QStringBuilder>

bool SolutionDraft::isEmpty() const
{
    return title.trimmed().isEmpty()
        && problemType.trimmed().isEmpty()
        && overview.trimmed().isEmpty()
        && algorithmAnalysis.trimmed().isEmpty();
}

QString SolutionDraft::toPlainText() const
{
    QString result;
    result += QStringLiteral("题目标题: ") + title + QLatin1Char('\n');
    result += QStringLiteral("题目类型: ") + problemType + QLatin1Char('\n');
    result += QStringLiteral("难度: ") + difficulty + QLatin1Char('\n');
    result += QStringLiteral("核心思路: ") + overview + QLatin1Char('\n');
    result += QStringLiteral("算法分析: ") + algorithmAnalysis + QLatin1Char('\n');
    result += QStringLiteral("时间复杂度: ") + timeComplexity + QLatin1Char('\n');
    result += QStringLiteral("空间复杂度: ") + spaceComplexity + QLatin1Char('\n');

    if (!cppFramework.isEmpty()) {
        result += QStringLiteral("C++ 伪代码/框架:\n");
        for (const QString& line : cppFramework) {
            result += QStringLiteral("  ") + line + QLatin1Char('\n');
        }
    }

    if (!examples.isEmpty()) {
        result += QStringLiteral("测试样例:\n");
        for (const ExampleCase& ex : examples) {
            result += QStringLiteral("  输入: ") + ex.input + QLatin1Char('\n');
            result += QStringLiteral("  输出: ") + ex.output + QLatin1Char('\n');
            result += QStringLiteral("  解释: ") + ex.explanation + QLatin1Char('\n');
        }
    }

    if (!commonMistakes.isEmpty()) {
        result += QStringLiteral("常见错误:\n");
        for (const QString& item : commonMistakes) {
            result += QStringLiteral("  - ") + item + QLatin1Char('\n');
        }
    }

    if (!assumptions.isEmpty()) {
        result += QStringLiteral("假设:\n");
        for (const QString& item : assumptions) {
            result += QStringLiteral("  - ") + item + QLatin1Char('\n');
        }
    }

    return result.trimmed();
}

