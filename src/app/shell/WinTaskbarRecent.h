#ifndef PIXELSTUDIO_APP_WINTASKBARRECENT_H
#define PIXELSTUDIO_APP_WINTASKBARRECENT_H

#include <QObject>
#include <QVariantList>

class WinTaskbarRecent : public QObject
{
    Q_OBJECT
public:
    explicit WinTaskbarRecent(QObject *parent = nullptr);

    static void installIdentity();

    Q_INVOKABLE void syncFromRecentFiles(const QVariantList &recent);
};

#endif // PIXELSTUDIO_APP_WINTASKBARRECENT_H
