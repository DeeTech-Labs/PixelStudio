#ifndef PIXELSTUDIO_APP_CODESYNTAXTHEME_H
#define PIXELSTUDIO_APP_CODESYNTAXTHEME_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariantMap>

class CodeSyntaxTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY changed)
    Q_PROPERTY(QString directive READ directive WRITE setDirective NOTIFY changed)
    Q_PROPERTY(QString keyword READ keyword WRITE setKeyword NOTIFY changed)
    Q_PROPERTY(QString type READ type WRITE setType NOTIFY changed)
    Q_PROPERTY(QString number READ number WRITE setNumber NOTIFY changed)
    Q_PROPERTY(QString string READ string WRITE setString NOTIFY changed)
    Q_PROPERTY(QString macro READ macro WRITE setMacro NOTIFY changed)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY changed)
    Q_PROPERTY(QString punctuation READ punctuation WRITE setPunctuation NOTIFY changed)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY changed)
    Q_PROPERTY(int revision READ revision NOTIFY changed)

public:
    explicit CodeSyntaxTheme(QObject *parent = nullptr);

    QString comment() const { return m_comment; }
    QString directive() const { return m_directive; }
    QString keyword() const { return m_keyword; }
    QString type() const { return m_type; }
    QString number() const { return m_number; }
    QString string() const { return m_string; }
    QString macro() const { return m_macro; }
    QString identifier() const { return m_identifier; }
    QString punctuation() const { return m_punctuation; }
    QString text() const { return m_text; }
    int revision() const { return m_revision; }

    void load(QSettings &settings);
    QVariantMap asMap() const;

    Q_INVOKABLE QString highlight(const QString &code) const;
    Q_INVOKABLE void resetDefaults();

public slots:
    void setComment(const QString &value);
    void setDirective(const QString &value);
    void setKeyword(const QString &value);
    void setType(const QString &value);
    void setNumber(const QString &value);
    void setString(const QString &value);
    void setMacro(const QString &value);
    void setIdentifier(const QString &value);
    void setPunctuation(const QString &value);
    void setText(const QString &value);

signals:
    void changed();

private:
    void setColor(QString &field, const QString &value, const char *settingsKey);
    void saveColor(const char *settingsKey, const QString &value);
    static QString normalizeColor(const QString &value, const QString &fallback);
    static void applyDefaults(CodeSyntaxTheme *theme);

    QString m_comment;
    QString m_directive;
    QString m_keyword;
    QString m_type;
    QString m_number;
    QString m_string;
    QString m_macro;
    QString m_identifier;
    QString m_punctuation;
    QString m_text;
    int m_revision = 0;
    QSettings *m_settings = nullptr;
};

#endif // PIXELSTUDIO_APP_CODESYNTAXTHEME_H
