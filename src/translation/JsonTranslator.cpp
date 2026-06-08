#include "translation/JsonTranslator.h"

#include "translation/TranslationStore.h"

JsonTranslator::JsonTranslator(const TranslationStore *store, QObject *parent)
    : QTranslator(parent)
    , m_store(store)
{
}

QString JsonTranslator::translate(const char *context,
                                  const char *sourceText,
                                  const char *disambiguation,
                                  int n) const
{
    Q_UNUSED(disambiguation);
    Q_UNUSED(n);
    if (!m_store || !context || !sourceText)
        return QString();

    const QString translated = m_store->translate(QString::fromUtf8(context),
                                                  QString::fromUtf8(sourceText));
    return translated.isEmpty() ? QString() : translated;
}
