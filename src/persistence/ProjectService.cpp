#include "i18n/AppLocale.h"
#include "persistence/ProjectFormat.h"
#include "persistence/ProjectService.h"

#include <QFile>
#include <QSaveFile>

StudioProject ProjectService::fromSession(const QString &name,
                                           const SessionSnapshot &session,
                                           int offsetX,
                                           int offsetY)
{
    StudioProject project;
    project.name = name.isEmpty() ? QStringLiteral("Untitled") : name;
    project.offsetX = offsetX;
    project.offsetY = offsetY;
    project.session = session;
    return project;
}

bool ProjectService::save(const StudioProject &project, const QUrl &url, QString *errorText)
{
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to save project: %1").arg(path);
        return false;
    }
    file.write(ProjectFormat::encode(project));
    if (!file.commit()) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to save project: %1").arg(path);
        return false;
    }
    return true;
}

bool ProjectService::load(const QUrl &url, StudioProject *project, QString *errorText)
{
    if (!project)
        return false;
    QString path = url.toLocalFile();
    if (path.isEmpty())
        path = url.path();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorText)
            *errorText = AppLocale::tr("Failed to open project: %1").arg(path);
        return false;
    }
    return ProjectFormat::decode(file.readAll(), project, errorText);
}

QVariantList ProjectService::assetsToVariantList(const QVector<ProjectAsset> &assets)
{
    QVariantList out;
    for (const ProjectAsset &asset : assets) {
        QVariantMap item;
        item.insert(QStringLiteral("path"), asset.path);
        item.insert(QStringLiteral("name"), asset.name);
        item.insert(QStringLiteral("offsetX"), asset.offsetX);
        item.insert(QStringLiteral("offsetY"), asset.offsetY);
        out.append(item);
    }
    return out;
}
