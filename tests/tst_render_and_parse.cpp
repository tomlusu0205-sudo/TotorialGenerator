#include "htmlrenderer.h"
#include "responseparser.h"

#include <QtTest/QtTest>

class RenderAndParseTest : public QObject
{
    Q_OBJECT

private slots:
    void parseAssistantContentFromChatCompletionsResponse();
    void parseAssistantContentHandlesErrorEnvelope();
    void parseSolutionDraftFromCodeFencedJson();
    void renderHtmlContainsMainSectionsAndEscapesText();
};

void RenderAndParseTest::parseAssistantContentFromChatCompletionsResponse()
{
    const QByteArray response = R"json(
    {
      "id": "chatcmpl-test",
      "object": "chat.completion",
      "choices": [
        {
          "index": 0,
          "message": {
            "role": "assistant",
            "content": "{\"title\":\"两数之和\",\"problemType\":\"哈希表\",\"difficulty\":\"简单\"}"
          },
          "finish_reason": "stop"
        }
      ]
    }
    )json";

    QString error;
    const QString content = ResponseParser::extractAssistantContent(response, &error);

    QCOMPARE(error, QString());
    QVERIFY(content.contains(QStringLiteral("\"title\":\"两数之和\"")));
    QVERIFY(content.contains(QStringLiteral("\"problemType\":\"哈希表\"")));
}

void RenderAndParseTest::parseAssistantContentHandlesErrorEnvelope()
{
    const QByteArray response = R"json(
    {
      "error": {
        "message": "Invalid API key",
        "type": "invalid_request_error"
      }
    }
    )json";

    QString error;
    const QString content = ResponseParser::extractAssistantContent(response, &error);

    QVERIFY(content.isEmpty());
    QCOMPARE(error, QStringLiteral("invalid_request_error: Invalid API key"));
}

void RenderAndParseTest::parseSolutionDraftFromCodeFencedJson()
{
    const QString wrappedJson = QStringLiteral(R"(
```json
{
  "title": "最长上升子序列",
  "problemType": "动态规划",
  "difficulty": "中等",
  "overview": "维护一个递增序列的最小结尾。",
  "algorithmAnalysis": "用二分更新 dp 数组。",
  "timeComplexity": "O(n log n)",
  "spaceComplexity": "O(n)",
  "cppFramework": [
    "vector<int> dp;",
    "for (int x : nums) { /* ... */ }"
  ],
  "examples": [
    {
      "input": "5\\n10 9 2 5 3",
      "output": "2",
      "explanation": "最长递增子序列为 2,5。"
    }
  ],
  "commonMistakes": [
    "把非严格递增和严格递增混淆"
  ],
  "assumptions": [
    "题目要求严格递增子序列"
  ]
}
```)");

    QString error;
    const SolutionDraft draft = ResponseParser::parseSolutionDraftJson(wrappedJson, &error);

    QCOMPARE(error, QString());
    QCOMPARE(draft.title, QStringLiteral("最长上升子序列"));
    QCOMPARE(draft.problemType, QStringLiteral("动态规划"));
    QCOMPARE(draft.difficulty, QStringLiteral("中等"));
    QCOMPARE(draft.cppFramework.size(), 2);
    QCOMPARE(draft.examples.size(), 1);
    QCOMPARE(draft.commonMistakes.size(), 1);
    QCOMPARE(draft.assumptions.size(), 1);
    QCOMPARE(draft.examples.first().output, QStringLiteral("2"));
}

void RenderAndParseTest::renderHtmlContainsMainSectionsAndEscapesText()
{
    SolutionDraft draft;
    draft.title = QStringLiteral("最小路径和 <test>");
    draft.problemType = QStringLiteral("动态规划");
    draft.difficulty = QStringLiteral("中等");
    draft.overview = QStringLiteral("从左上到右下逐步转移。\n\n注意边界。");
    draft.algorithmAnalysis = QStringLiteral("用 dp[i][j] 表示到当前位置的最小代价。");
    draft.timeComplexity = QStringLiteral("O(mn)");
    draft.spaceComplexity = QStringLiteral("O(n)");
    draft.cppFramework = {
        QStringLiteral("vector<int> dp(n, INF);"),
        QStringLiteral("dp[0] = 0;")
    };
    draft.examples = {
        ExampleCase{QStringLiteral("3 3\\n1 3 1"), QStringLiteral("7"), QStringLiteral("路径 1->3->1。")}
    };
    draft.commonMistakes = { QStringLiteral("忘记处理首行首列") };
    draft.assumptions = { QStringLiteral("网格权值为非负") };

    const QString html = HtmlRenderer::render(draft);

    QVERIFY(html.contains(QStringLiteral("核心思路")));
    QVERIFY(html.contains(QStringLiteral("算法分析")));
    QVERIFY(html.contains(QStringLiteral("C++ 算法框架 / 伪代码")));
    QVERIFY(html.contains(QStringLiteral("测试样例")));
    QVERIFY(html.contains(QStringLiteral("常见错误")));
    QVERIFY(html.contains(QStringLiteral("最小路径和 &lt;test&gt;")));
    QVERIFY(html.contains(QStringLiteral("vector&lt;int&gt; dp(n, INF);")));
    QVERIFY(html.contains(QStringLiteral("样例 1")));
    QVERIFY(html.contains(QStringLiteral("路径 1-&gt;3-&gt;1。")));
}

QTEST_MAIN(RenderAndParseTest)
#include "tst_render_and_parse.moc"
