#ifndef PIXELSTUDIO_APP_CODESYNTAXHIGHLIGHTER_H
#define PIXELSTUDIO_APP_CODESYNTAXHIGHLIGHTER_H

#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QPair>

class CodeSyntaxTheme;

class CodeSyntaxHighlighter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QString highlightedHtml READ highlightedHtml NOTIFY highlightedHtmlChanged)
    Q_PROPERTY(bool highlighting READ highlighting NOTIFY highlightingChanged)
    Q_PROPERTY(int lineCount READ lineCount NOTIFY lineCountChanged)

public:
    explicit CodeSyntaxHighlighter(QObject *parent = nullptr);

    QString sourceText() const { return m_sourceText; }
    QString highlightedHtml() const { return m_highlightedHtml; }
    bool highlighting() const { return m_highlighting; }
    int lineCount() const { return m_lineCount; }

    Q_INVOKABLE void setTheme(CodeSyntaxTheme *theme);
    void setSourceText(const QString &text);

signals:
    void sourceTextChanged();
    void highlightedHtmlChanged();
    void highlightingChanged();
    void lineCountChanged();

private:
    void scheduleHighlight();
    void startHighlight();
    void applyHighlightResult(const QPair<quint64, QString> &result);
    static int countLines(const QString &text);

    CodeSyntaxTheme *m_theme = nullptr;
    QString m_sourceText;
    QString m_highlightedHtml;
    bool m_highlighting = false;
    int m_lineCount = 0;
    quint64 m_requestId = 0;
    QTimer m_debounceTimer;
    QFutureWatcher<QPair<quint64, QString>> m_highlightWatcher;
};

#endif // PIXELSTUDIO_APP_CODESYNTAXHIGHLIGHTER_H
