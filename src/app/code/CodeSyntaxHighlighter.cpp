#include "app/code/CodeSyntaxHighlighter.h"
#include "app/code/CodeSyntaxTheme.h"

#include <QtConcurrent>

namespace {
constexpr int kDebounceMs = 48;
}

CodeSyntaxHighlighter::CodeSyntaxHighlighter(QObject *parent)
    : QObject(parent)
{
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(kDebounceMs);
    connect(&m_debounceTimer, &QTimer::timeout, this, &CodeSyntaxHighlighter::startHighlight);
    connect(&m_highlightWatcher,
            &QFutureWatcher<QPair<quint64, QString>>::finished,
            this,
            [this]() { applyHighlightResult(m_highlightWatcher.result()); });
}

void CodeSyntaxHighlighter::setTheme(CodeSyntaxTheme *theme)
{
    if (m_theme == theme)
        return;
    if (m_theme)
        disconnect(m_theme, &CodeSyntaxTheme::changed, this, nullptr);
    m_theme = theme;
    if (m_theme)
        connect(m_theme, &CodeSyntaxTheme::changed, this, &CodeSyntaxHighlighter::scheduleHighlight);
    scheduleHighlight();
}

void CodeSyntaxHighlighter::setSourceText(const QString &text)
{
    if (m_sourceText == text)
        return;
    m_sourceText = text;
    const int nextLineCount = countLines(text);
    if (m_lineCount != nextLineCount) {
        m_lineCount = nextLineCount;
        emit lineCountChanged();
    }
    emit sourceTextChanged();
    scheduleHighlight();
}

int CodeSyntaxHighlighter::countLines(const QString &text)
{
    if (text.isEmpty())
        return 0;
    return text.count(QLatin1Char('\n')) + 1;
}

void CodeSyntaxHighlighter::scheduleHighlight()
{
    if (!m_theme) {
        if (!m_highlightedHtml.isEmpty()) {
            m_highlightedHtml.clear();
            emit highlightedHtmlChanged();
        }
        return;
    }
    m_debounceTimer.start();
}

void CodeSyntaxHighlighter::startHighlight()
{
    if (!m_theme) {
        if (m_highlighting) {
            m_highlighting = false;
            emit highlightingChanged();
        }
        return;
    }

    if (m_highlightWatcher.isRunning())
        ++m_requestId;

    const QString code = m_sourceText;
    const quint64 requestId = ++m_requestId;
    CodeSyntaxTheme *theme = m_theme;

    if (!m_highlighting) {
        m_highlighting = true;
        emit highlightingChanged();
    }

    auto future = QtConcurrent::run([theme, code, requestId]() {
        return qMakePair(requestId, theme->highlight(code));
    });
    m_highlightWatcher.setFuture(future);
}

void CodeSyntaxHighlighter::applyHighlightResult(const QPair<quint64, QString> &result)
{
    if (result.first != m_requestId)
        return;

    if (m_highlightedHtml != result.second) {
        m_highlightedHtml = result.second;
        emit highlightedHtmlChanged();
    }
    if (m_highlighting) {
        m_highlighting = false;
        emit highlightingChanged();
    }
}
