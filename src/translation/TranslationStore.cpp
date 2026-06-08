#include "translation/TranslationStore.h"

#include "translation/TranslationStoreBuiltin.h"
#include "persistence/AppPaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QVariantMap>

namespace {

QString systemLocaleCode()
{
    const QStringList langs = QLocale::system().uiLanguages();
    if (!langs.isEmpty())
        return langs.first().section(QLatin1Char('-'), 0, 0).toLower();
    return QLocale::system().bcp47Name().section(QLatin1Char('-'), 0, 0).toLower();
}

bool readJsonFile(const QString &path, QJsonObject *out)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return false;
    *out = doc.object();
    return true;
}

bool isExampleBundleFile(const QString &fileName)
{
    return fileName.endsWith(QStringLiteral(".example.json"), Qt::CaseInsensitive);
}

QStringList contributorsFromJson(const QJsonValue &value)
{
    QStringList contributors;
    if (!value.isArray())
        return contributors;
    for (const QJsonValue &entry : value.toArray()) {
        if (!entry.isString())
            continue;
        const QString name = entry.toString().trimmed();
        if (!name.isEmpty() && !contributors.contains(name))
            contributors.append(name);
    }
    return contributors;
}

void mergeContributorLists(QStringList *target, const QStringList &extra)
{
    if (!target)
        return;
    for (const QString &name : extra) {
        if (!name.isEmpty() && !target->contains(name))
            target->append(name);
    }
}

TranslationStore::LanguageInfo catalogEntryFromJson(const QString &path, bool userSupplied)
{
    QJsonObject root;
    if (!readJsonFile(path, &root))
        return {};

    QString code = root.value(QStringLiteral("language")).toString().trimmed().toLower();
    if (code.isEmpty())
        code = QFileInfo(path).completeBaseName().toLower();

    QString name = root.value(QStringLiteral("name")).toString().trimmed();
    if (code.isEmpty())
        return {};

    TranslationStore::LanguageInfo info;
    info.code = code;
    info.name = name.isEmpty() ? code : name;
    info.author = root.value(QStringLiteral("author")).toString().trimmed();
    info.contributors = contributorsFromJson(root.value(QStringLiteral("contributors")));
    info.userSupplied = userSupplied;
    return info;
}

} // namespace

TranslationStore &TranslationStore::instance()
{
    static TranslationStore store;
    return store;
}

void TranslationStore::refreshCatalog()
{
    m_catalog.clear();

    for (const char *const *code = TranslationStoreBuiltin::kBundledLanguageCodes; *code; ++code) {
        const QString path = QStringLiteral(":/translations/%1.json").arg(QString::fromUtf8(*code));
        const LanguageInfo info = catalogEntryFromJson(path, false);
        if (!info.code.isEmpty())
            appendCatalogEntry(info);
    }

    const QString userDir = AppPaths::userTranslationsDir();
    QDir dir(userDir);
    if (!dir.exists())
        return;

    const QStringList files = dir.entryList({QStringLiteral("*.json")}, QDir::Files);
    for (const QString &fileName : files) {
        if (isExampleBundleFile(fileName))
            continue;
        const LanguageInfo info = catalogEntryFromJson(userDir + QLatin1Char('/') + fileName, true);
        if (!info.code.isEmpty())
            appendCatalogEntry(info);
    }
}

bool TranslationStore::hasLanguage(const QString &code) const
{
    const QString normalized = code.toLower();
    for (const LanguageInfo &info : m_catalog) {
        if (info.code == normalized)
            return true;
    }
    return QFile::exists(QStringLiteral(":/translations/%1.json").arg(normalized))
        || QFile::exists(AppPaths::userTranslationsDir() + QLatin1Char('/') + normalized
                         + QStringLiteral(".json"));
}

QString TranslationStore::effectiveLanguageCode(const QString &languageCode) const
{
    if (languageCode == QStringLiteral("system")) {
        const QString system = systemLocaleCode();
        if (hasLanguage(system))
            return system;
        return hasLanguage(QStringLiteral("en")) ? QStringLiteral("en")
                                                 : QStringLiteral("en");
    }

    const QString normalized = languageCode.toLower();
    if (hasLanguage(normalized))
        return normalized;
    return hasLanguage(QStringLiteral("en")) ? QStringLiteral("en") : normalized;
}

bool TranslationStore::loadLanguage(const QString &languageCode)
{
    const QString effectiveCode = effectiveLanguageCode(languageCode);
    clearEntries();

    const QString builtinPath = QStringLiteral(":/translations/%1.json").arg(effectiveCode);
    const QString userPath = AppPaths::userTranslationsDir() + QLatin1Char('/') + effectiveCode
                             + QStringLiteral(".json");

    bool loaded = false;
    if (QFile::exists(builtinPath))
        loaded = mergeJsonFile(builtinPath, true) || loaded;
    if (QFile::exists(userPath))
        loaded = mergeJsonFile(userPath, false) || loaded;

    m_activeCode = effectiveCode;
    m_activeName = readLanguageName(effectiveCode, effectiveCode);
    return loaded || effectiveCode == QStringLiteral("en");
}

QString TranslationStore::translate(const QString &context, const QString &source) const
{
    const auto contextIt = m_entries.constFind(context);
    if (contextIt == m_entries.cend())
        return QString();
    const auto sourceIt = contextIt->constFind(source);
    if (sourceIt == contextIt->cend())
        return QString();
    return *sourceIt;
}

QVariantList TranslationStore::availableLanguages() const
{
    QVariantList list;
    const QString systemLabel = translate(QStringLiteral("Core"), QStringLiteral("System language"));
    list.append(QVariantMap{{QStringLiteral("code"), QStringLiteral("system")},
                            {QStringLiteral("name"),
                             systemLabel.isEmpty() ? QStringLiteral("System language") : systemLabel}});
    for (const LanguageInfo &info : m_catalog) {
        if (info.code == QStringLiteral("system"))
            continue;
        const QString name = info.name.isEmpty() ? info.code : info.name;
        QVariantList contributorList;
        contributorList.reserve(info.contributors.size());
        for (const QString &contributor : info.contributors)
            contributorList.append(contributor);

        list.append(QVariantMap{{QStringLiteral("code"), info.code},
                                {QStringLiteral("name"), name},
                                {QStringLiteral("author"), info.author},
                                {QStringLiteral("contributors"), contributorList},
                                {QStringLiteral("userSupplied"), info.userSupplied}});
    }
    return list;
}

void TranslationStore::clearEntries()
{
    m_entries.clear();
    m_activeCode.clear();
    m_activeName.clear();
    m_activeAuthor.clear();
    m_activeContributors.clear();
}

bool TranslationStore::mergeJsonFile(const QString &path, bool replaceExisting)
{
    QJsonObject root;
    if (!readJsonFile(path, &root))
        return false;
    return mergeJsonObject(root, replaceExisting);
}

bool TranslationStore::mergeJsonObject(const QJsonObject &root, bool replaceExisting)
{
    const QString language = root.value(QStringLiteral("language")).toString();
    if (!language.isEmpty())
        m_activeCode = language.toLower();

    const QString name = root.value(QStringLiteral("name")).toString().trimmed();
    if (!name.isEmpty())
        m_activeName = name;

    const QString author = root.value(QStringLiteral("author")).toString().trimmed();
    const QStringList contributors = contributorsFromJson(root.value(QStringLiteral("contributors")));
    if (replaceExisting) {
        if (!author.isEmpty())
            m_activeAuthor = author;
        if (!contributors.isEmpty())
            m_activeContributors = contributors;
    } else {
        if (!author.isEmpty())
            m_activeAuthor = author;
        mergeContributorLists(&m_activeContributors, contributors);
    }

    const QJsonObject contexts = root.value(QStringLiteral("contexts")).toObject();
    if (contexts.isEmpty())
        return false;

    for (auto contextIt = contexts.begin(); contextIt != contexts.end(); ++contextIt) {
        const QString context = contextIt.key();
        const QJsonObject messages = contextIt.value().toObject();
        QHash<QString, QString> &bucket = m_entries[context];
        for (auto messageIt = messages.begin(); messageIt != messages.end(); ++messageIt) {
            const QString source = messageIt.key();
            const QString translation = messageIt.value().toString();
            if (replaceExisting || !bucket.contains(source))
                bucket.insert(source, translation);
        }
    }

    return true;
}

void TranslationStore::appendCatalogEntry(const LanguageInfo &info)
{
    if (info.code.isEmpty())
        return;
    for (LanguageInfo &existing : m_catalog) {
        if (existing.code != info.code)
            continue;
        if (info.userSupplied) {
            existing.userSupplied = true;
            if (!info.name.isEmpty())
                existing.name = info.name;
            if (!info.author.isEmpty())
                existing.author = info.author;
            mergeContributorLists(&existing.contributors, info.contributors);
        } else {
            if (existing.name.isEmpty() && !info.name.isEmpty())
                existing.name = info.name;
            if (existing.author.isEmpty() && !info.author.isEmpty())
                existing.author = info.author;
            if (existing.contributors.isEmpty())
                existing.contributors = info.contributors;
        }
        return;
    }

    LanguageInfo entry = info;
    if (entry.name.isEmpty())
        entry.name = entry.code;
    m_catalog.append(entry);
}

QString TranslationStore::readLanguageName(const QString &code,
                                           const QString &fallback) const
{
    for (const LanguageInfo &info : m_catalog) {
        if (info.code == code && !info.name.isEmpty())
            return info.name;
    }

    const QString builtinPath = QStringLiteral(":/translations/%1.json").arg(code);
    const LanguageInfo builtin = catalogEntryFromJson(builtinPath, false);
    if (!builtin.name.isEmpty())
        return builtin.name;

    const QString userPath = AppPaths::userTranslationsDir() + QLatin1Char('/') + code
                             + QStringLiteral(".json");
    const LanguageInfo user = catalogEntryFromJson(userPath, true);
    if (!user.name.isEmpty())
        return user.name;

    return fallback;
}
