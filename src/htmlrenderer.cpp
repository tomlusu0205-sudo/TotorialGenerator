#include "htmlrenderer.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QStringBuilder>

namespace {

QString joinParagraphs(const QString& text)
{
    if (text.trimmed().isEmpty()) {
        return QStringLiteral("<p class=\"muted\">暂无内容</p>");
    }

    const QStringList paragraphs = text.trimmed().split(QRegularExpression(QStringLiteral("\\n\\s*\\n")), Qt::SkipEmptyParts);
    QString html;
    for (const QString& paragraph : paragraphs) {
        html += QStringLiteral("<p>") + paragraph.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>")) + QStringLiteral("</p>");
    }
    return html;
}

QString listToHtml(const QStringList& items)
{
    if (items.isEmpty()) {
        return QStringLiteral("<p class=\"muted\">暂无内容</p>");
    }

    QString html = QStringLiteral("<ul class=\"bullet-list\">");
    for (const QString& item : items) {
        html += QStringLiteral("<li>") + item.toHtmlEscaped() + QStringLiteral("</li>");
    }
    html += QStringLiteral("</ul>");
    return html;
}

QString codeBlockToHtml(const QStringList& lines)
{
    if (lines.isEmpty()) {
        return QStringLiteral("<p class=\"muted\">暂无内容</p>");
    }

    QString html = QStringLiteral("<pre class=\"code-block\"><code>");
    html += lines.join(QLatin1Char('\n')).toHtmlEscaped();
    html += QStringLiteral("</code></pre>");
    return html;
}

QString examplesToHtml(const QVector<ExampleCase>& examples)
{
    if (examples.isEmpty()) {
        return QStringLiteral("<p class=\"muted\">暂无内容</p>");
    }

    QString html;
    for (int i = 0; i < examples.size(); ++i) {
        const ExampleCase& ex = examples.at(i);
        html += QStringLiteral("<div class=\"example-card\">");
        html += QStringLiteral("<div class=\"example-title\">样例 ") + QString::number(i + 1) + QStringLiteral("</div>");
        html += QStringLiteral("<div class=\"example-grid\">");
        html += QStringLiteral("<div><div class=\"label\">输入</div><pre>") + ex.input.toHtmlEscaped() + QStringLiteral("</pre></div>");
        html += QStringLiteral("<div><div class=\"label\">输出</div><pre>") + ex.output.toHtmlEscaped() + QStringLiteral("</pre></div>");
        html += QStringLiteral("</div>");
        if (!ex.explanation.trimmed().isEmpty()) {
            html += QStringLiteral("<div class=\"label\">解释</div><p>") + ex.explanation.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>")) + QStringLiteral("</p>");
        }
        html += QStringLiteral("</div>");
    }
    return html;
}

} // namespace

namespace HtmlRenderer {

QString render(const SolutionDraft& draft)
{
    const QString generatedAt = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));

    QString html;
    html += QStringLiteral(R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0" />
<title>)");
    html += (draft.title.trimmed().isEmpty() ? QStringLiteral("题解") : draft.title.toHtmlEscaped());
    html += QStringLiteral(R"(</title>
<style>
:root {
    color-scheme: light;
    --bg: #f5f7fb;
    --card: #ffffff;
    --text: #1f2937;
    --muted: #6b7280;
    --primary: #2563eb;
    --primary-soft: #dbeafe;
    --border: #e5e7eb;
    --code-bg: #0f172a;
    --code-text: #e2e8f0;
}
* { box-sizing: border-box; }
body {
    margin: 0;
    padding: 0;
    font-family: "Segoe UI", "PingFang SC", "Microsoft YaHei", sans-serif;
    background:
        radial-gradient(circle at top left, rgba(37, 99, 235, 0.10), transparent 25%),
        radial-gradient(circle at top right, rgba(14, 165, 233, 0.10), transparent 22%),
        var(--bg);
    color: var(--text);
}
.page {
    max-width: 1024px;
    margin: 0 auto;
    padding: 32px 20px 48px;
}
.hero {
    background: linear-gradient(135deg, #ffffff 0%, #f8fbff 100%);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 24px 28px;
    box-shadow: 0 20px 45px rgba(15, 23, 42, 0.08);
}
.title {
    margin: 0 0 10px;
    font-size: 30px;
    line-height: 1.2;
    color: #0f172a;
}
.meta {
    display: flex;
    flex-wrap: wrap;
    gap: 10px;
    color: var(--muted);
    font-size: 14px;
}
.badge {
    display: inline-block;
    padding: 6px 12px;
    border-radius: 999px;
    background: var(--primary-soft);
    color: var(--primary);
    font-weight: 600;
}
.section {
    margin-top: 20px;
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 18px;
    padding: 22px 24px;
}
.section h2 {
    margin: 0 0 14px;
    font-size: 20px;
    color: #111827;
    border-left: 4px solid var(--primary);
    padding-left: 12px;
}
p {
    margin: 0 0 12px;
    line-height: 1.75;
    white-space: normal;
}
.muted {
    color: var(--muted);
}
.bullet-list {
    margin: 0;
    padding-left: 20px;
    line-height: 1.8;
}
.bullet-list li {
    margin-bottom: 8px;
}
.code-block {
    margin: 0;
    padding: 18px 20px;
    background: var(--code-bg);
    color: var(--code-text);
    border-radius: 14px;
    overflow-x: auto;
    white-space: pre-wrap;
    word-break: break-word;
}
.example-card {
    border: 1px solid var(--border);
    border-radius: 16px;
    padding: 18px;
    margin-bottom: 16px;
    background: #fafcff;
}
.example-title {
    font-weight: 700;
    margin-bottom: 12px;
    color: #0f172a;
}
.example-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 14px;
}
.label {
    font-weight: 700;
    color: var(--primary);
    margin-bottom: 8px;
}
pre {
    margin: 0;
    padding: 14px 16px;
    border-radius: 12px;
    background: #eef2ff;
    color: #1e293b;
    overflow-x: auto;
    white-space: pre-wrap;
    word-break: break-word;
}
@media (max-width: 720px) {
    .page { padding: 16px; }
    .hero, .section { padding: 18px; border-radius: 16px; }
    .title { font-size: 24px; }
    .example-grid { grid-template-columns: 1fr; }
}
</style>
</head>
<body>
<div class="page">
    <div class="hero">
        <h1 class="title">)");
    html += (draft.title.trimmed().isEmpty() ? QStringLiteral("自动生成题解") : draft.title.toHtmlEscaped());
    html += QStringLiteral(R"(</h1>
        <div class="meta">
            <span class="badge">)");
    html += (draft.problemType.trimmed().isEmpty() ? QStringLiteral("题目类型待确认") : draft.problemType.toHtmlEscaped());
    html += QStringLiteral(R"(</span>
            <span>生成时间：)");
    html += generatedAt.toHtmlEscaped();
    html += QStringLiteral(R"(</span>
            <span>)");
    html += (draft.difficulty.trimmed().isEmpty() ? QStringLiteral("难度：未知") : QStringLiteral("难度：") + draft.difficulty.toHtmlEscaped());
    html += QStringLiteral(R"(</span>
        </div>
    </div>

    <div class="section">
        <h2>核心思路</h2>
        )");
    html += joinParagraphs(draft.overview);
    html += QStringLiteral(R"(    </div>

    <div class="section">
        <h2>算法分析</h2>
        )");
    html += joinParagraphs(draft.algorithmAnalysis);
    html += QStringLiteral(R"(    </div>

    <div class="section">
        <h2>复杂度</h2>
        <p><strong>时间复杂度：</strong>)");
    html += (draft.timeComplexity.trimmed().isEmpty() ? QStringLiteral("待补充") : draft.timeComplexity.toHtmlEscaped());
    html += QStringLiteral(R"(</p>
        <p><strong>空间复杂度：</strong>)");
    html += (draft.spaceComplexity.trimmed().isEmpty() ? QStringLiteral("待补充") : draft.spaceComplexity.toHtmlEscaped());
    html += QStringLiteral(R"(</p>
    </div>

    <div class="section">
        <h2>C++ 算法框架 / 伪代码</h2>
        )");
    html += codeBlockToHtml(draft.cppFramework);
    html += QStringLiteral(R"(    </div>

    <div class="section">
        <h2>测试样例</h2>
        )");
    html += examplesToHtml(draft.examples);
    html += QStringLiteral(R"(    </div>

    <div class="section">
        <h2>常见错误</h2>
        )");
    html += listToHtml(draft.commonMistakes);
    html += QStringLiteral(R"(    </div>

    <div class="section">
        <h2>假设与补充说明</h2>
        )");
    html += listToHtml(draft.assumptions);
    html += QStringLiteral(R"(    </div>
</div>
</body>
</html>)");

    return html;
}

} // namespace HtmlRenderer
