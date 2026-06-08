#ifndef PIXELSTUDIO_TRANSLATION_JSONTRANSLATOR_H
#define PIXELSTUDIO_TRANSLATION_JSONTRANSLATOR_H

#include <QTranslator>

class TranslationStore;

class JsonTranslator : public QTranslator
{
public:
    explicit JsonTranslator(const TranslationStore *store, QObject *parent = nullptr);

    QString translate(const char *context,
                      const char *sourceText,
                      const char *disambiguation = nullptr,
                      int n = -1) const override;

private:
    const TranslationStore *m_store = nullptr;
};

#endif // PIXELSTUDIO_TRANSLATION_JSONTRANSLATOR_H
