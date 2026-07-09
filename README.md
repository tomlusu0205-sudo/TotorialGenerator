# C++ 题目描述到题解生成器

一个基于 Qt Widgets 的桌面应用：输入题目描述后，调用 OpenAI API 自动生成结构化题解，并支持 HTML 预览与导出。

## 功能

- 左侧输入题目描述，右侧实时预览题解
- 支持生成、停止、复制、导出 HTML
- 支持配置 API Key、模型名、温度、最大输出长度、API Base URL
- 生成结果包含：
  - 题目类型
  - 解题思路
  - 算法复杂度
  - C++ 伪代码或框架
  - 测试样例
  - 常见错误

## 构建

需要本机安装 Qt 6（或 Qt 5）与 CMake。

```bash
cmake -S . -B build
cmake --build build
```

## 测试

项目带了一个本地单元测试，用来验证：

- OpenAI 返回包中的 assistant content 提取
- 题解 JSON 解析
- HTML 生成结果中的关键章节和转义行为

运行：

```bash
ctest --test-dir build
```

如果 Qt 没有自动被 CMake 找到，可以手动指定 `CMAKE_PREFIX_PATH`，例如：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.6.0/msvc2019_64"
```

## 运行

启动后在“设置”里填写 OpenAI API Key，默认模型可直接使用 `gpt-4.1-mini`，也可以改成你自己的模型名。

建议同时在环境变量中设置：

- `OPENAI_API_KEY`

程序会优先读取界面里保存的 API Key，若为空则回退到环境变量。
