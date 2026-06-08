#ifndef PIXELSTUDIO_TRANSLATION_TRANSLATIONSTORE_H
#define PIXELSTUDIO_TRANSLATION_TRANSLATIONSTORE_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVector>

class TranslationStore
{
public:
    struct LanguageInfo {
        QString code;
        QString name;
        QString author;
        QStringList contributors;
        bool userSupplied = false;
    };

    static TranslationStore &instance();

    void refreshCatalog();
    bool loadLanguage(const QString &languageCode);
    QString translate(const QString &context, const QString &source) const;

    QString activeLanguageCode() const { return m_activeCode; }
    QString activeLanguageName() const { return m_activeName; }
    QString activeAuthor() const { return m_activeAuthor; }
    QStringList activeContributors() const { return m_activeContributors; }
    QVector<LanguageInfo> catalog() const { return m_catalog; }
    QVariantList availableLanguages() const;
    bool hasLanguage(const QString &code) const;
    QString effectiveLanguageCode(const QString &languageCode) const;

private:
    TranslationStore() = default;

    void clearEntries();
    bool mergeJsonFile(const QString &path, bool replaceExisting);
    bool mergeJsonObject(const QJsonObject &root, bool replaceExisting);
    void appendCatalogEntry(const LanguageInfo &info);
    QString readLanguageName(const QString &code, const QString &fallback = QString()) const;

    QString m_activeCode;
    QString m_activeName;
    QString m_activeAuthor;
    QStringList m_activeContributors;
    QVector<LanguageInfo> m_catalog;
    QHash<QString, QHash<QString, QString>> m_entries;
};

#endif // PIXELSTUDIO_TRANSLATION_TRANSLATIONSTORE_H
