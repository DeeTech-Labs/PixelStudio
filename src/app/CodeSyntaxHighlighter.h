#ifndef PIXELSTUDIO_APP_CODESYNTAXHIGHLIGHTER_H
#define PIXELSTUDIO_APP_CODESYNTAXHIGHLIGHTER_H

#include <QObject>
#include <QString>
#include <QTimer>

class CodeSyntaxTheme;

class CodeSyntaxHighlighter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QString highlightedHtml READ highlightedHtml NOTIFY highlightedHtmlChanged)
    Q_PROPERTY(bool highlighting READ highlighting NOTIFY highlightingChanged)

public:
    explicit CodeSyntaxHighlighter(QObject *parent = nullptr);

    QString sourceText() const { return m_sourceText; }
    QString highlightedHtml() const { return m_highlightedHtml; }
    bool highlighting() const { return m_highlighting; }

    Q_INVOKABLE void setTheme(CodeSyntaxTheme *theme);
    void setSourceText(const QString &text);

signals:
    void sourceTextChanged();
    void highlightedHtmlChanged();
    void highlightingChanged();

private:
    void scheduleHighlight();
    void startHighlight();

    CodeSyntaxTheme *m_theme = nullptr;
    QString m_sourceText;
    QString m_highlightedHtml;
    bool m_highlighting = false;
    quint64 m_requestId = 0;
    QTimer m_debounceTimer;
};

#endif // PIXELSTUDIO_APP_CODESYNTAXHIGHLIGHTER_H
