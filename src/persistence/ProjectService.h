#ifndef PIXELSTUDIO_PERSISTENCE_PROJECTSERVICE_H
#define PIXELSTUDIO_PERSISTENCE_PROJECTSERVICE_H

#include <QString>
#include <QUrl>
#include <QVector>
#include <QVariantList>

#include "persistence/SessionSettings.h"

struct ProjectAsset
{
    QString path;
    QString name;
    int offsetX = 0;
    int offsetY = 0;
};

struct StudioProject
{
    QString name = QStringLiteral("Untitled");
    int offsetX = 0;
    int offsetY = 0;
    SessionSnapshot session;
    QVector<ProjectAsset> assets;
};

class ProjectService
{
public:
    static StudioProject fromSession(const QString &name,
                                     const SessionSnapshot &session,
                                     int offsetX,
                                     int offsetY);
    static bool save(const StudioProject &project, const QUrl &url, QString *errorText = nullptr);
    static bool load(const QUrl &url, StudioProject *project, QString *errorText = nullptr);
    static QVariantList assetsToVariantList(const QVector<ProjectAsset> &assets);
};

#endif // PIXELSTUDIO_PERSISTENCE_PROJECTSERVICE_H
