#include "app/CodeSyntaxHighlighter.h"
#include "app/CodeSyntaxTheme.h"

namespace {
constexpr int kDebounceMs = 48;
}

CodeSyntaxHighlighter::CodeSyntaxHighlighter(QObject *parent)
    : QObject(parent)
{
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(kDebounceMs);
    connect(&m_debounceTimer, &QTimer::timeout, this, &CodeSyntaxHighlighter::startHighlight);
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
    emit sourceTextChanged();
    scheduleHighlight();
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

    const QString code = m_sourceText;
    const quint64 requestId = ++m_requestId;

    if (!m_highlighting) {
        m_highlighting = true;
        emit highlightingChanged();
    }

    const QString html = m_theme->highlight(code);
    if (requestId != m_requestId)
        return;

    if (m_highlightedHtml != html) {
        m_highlightedHtml = html;
        emit highlightedHtmlChanged();
    }
    if (m_highlighting) {
        m_highlighting = false;
        emit highlightingChanged();
    }
}
