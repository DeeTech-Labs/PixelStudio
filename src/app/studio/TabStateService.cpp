#include "app/studio/TabStateService.h"

#include "app/preview/PreviewImageProvider.h"
#include "app/studio/DisplayConverter.h"
#include "app/studio/model/ConverterState.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "app/studio/pipeline/ImagePipelineController.h"
#include "app/tabs/TabPreviewSlots.h"
#include "persistence/ProjectService.h"

#include <QFileInfo>

namespace {

QUrl publishSlot(PreviewImageProvider *provider, const QString &slot, const QImage &image)
{
    if (!provider || slot.isEmpty() || image.isNull())
        return {};
    provider->setImage(slot, image);
    return provider->imageUrl(slot);
}

QUrl resolvePreviewUrl(PreviewImageProvider *provider, const QString &slot, const QImage &image)
{
    if (provider && provider->hasSlot(slot))
        return provider->imageUrl(slot);
    return publishSlot(provider, slot, image);
}

void hydrateSourceImage(ConverterTabSnapshot *snapshot)
{
    if (!snapshot || !snapshot->sourceImage.isNull())
        return;
    if (!snapshot->project.sourceImagePng.isEmpty()) {
        snapshot->sourceImage.loadFromData(snapshot->project.sourceImagePng, "PNG");
        return;
    }
    if (!snapshot->sourceFilePath.isEmpty() && QFileInfo::exists(snapshot->sourceFilePath)) {
        snapshot->sourceImage.load(snapshot->sourceFilePath);
        return;
    }
    const QString assetPath = ProjectService::primaryImagePath(snapshot->project);
    if (!assetPath.isEmpty() && QFileInfo::exists(assetPath)) {
        snapshot->sourceImage.load(assetPath);
        snapshot->sourceFilePath = assetPath;
    }
}

} // namespace

void TabStateService::publishSnapshotPreviews(PreviewImageProvider *provider,
                                              ConverterTabSnapshot *snapshot,
                                              const QImage &orientedSource)
{
    if (!provider || !snapshot || snapshot->tabId.isEmpty())
        return;

    hydrateSourceImage(snapshot);

    const QImage &sourceForSlot = !orientedSource.isNull() ? orientedSource : snapshot->sourceImage;
    if (!sourceForSlot.isNull()) {
        snapshot->sourcePath = publishSlot(provider,
                                           TabPreviewSlots::source(snapshot->tabId),
                                           sourceForSlot);
    }
    if (snapshot->hasPipelineResult && !snapshot->lastResult.preview.isNull()) {
        snapshot->previewPath = publishSlot(provider,
                                            TabPreviewSlots::preview(snapshot->tabId),
                                            snapshot->lastResult.preview);
        snapshot->processPreviewPath = publishSlot(provider,
                                                   TabPreviewSlots::process(snapshot->tabId),
                                                   snapshot->lastResult.processPreview);
    }
}

ConverterTabSnapshot TabStateService::capture(const DisplayConverter &converter, const QString &tabId)
{
    const ConverterState &state = converter.converterState();
    ConverterTabSnapshot snapshot;
    snapshot.tabId = tabId;
    snapshot.project = ProjectService::fromSession(state.project.name,
                                                   converter.sessionSnapshot(),
                                                   state.offsetX,
                                                   state.offsetY);
    snapshot.project.assets.clear();
    snapshot.project.sourceImagePng.clear();
    snapshot.project.resultPreviewPng.clear();

    const bool hasSourceFile = !state.sourceFilePath.isEmpty()
        && QFileInfo::exists(state.sourceFilePath);
    if (hasSourceFile) {
        snapshot.project.assets.append(ProjectAsset{
            state.sourceFilePath,
            QFileInfo(state.sourceFilePath).fileName(),
            0,
            0,
        });
    }

    snapshot.sourceImage = state.sourceImage;
    snapshot.sourceFilePath = state.sourceFilePath;
    snapshot.projectFileUrl = state.projectFile;
    snapshot.lastResult = state.lastResult;
    snapshot.generatedCode = state.generatedCode;
    snapshot.hasPipelineResult = !state.lastResult.preview.isNull();
    snapshot.sourcePath = state.sourcePath;
    snapshot.previewPath = state.previewPath;
    snapshot.processPreviewPath = state.processPreviewPath;

    if (PreviewImageProvider *provider = converter.previewProvider(); provider && !tabId.isEmpty()) {
        const QString sourceSlot = TabPreviewSlots::source(tabId);
        const bool previewsReady = provider->hasSlot(sourceSlot)
            && (!snapshot.hasPipelineResult || provider->hasSlot(TabPreviewSlots::preview(tabId)));
        if (previewsReady) {
            snapshot.sourcePath = provider->imageUrl(sourceSlot);
            if (snapshot.hasPipelineResult) {
                snapshot.previewPath = provider->imageUrl(TabPreviewSlots::preview(tabId));
                snapshot.processPreviewPath = provider->imageUrl(TabPreviewSlots::process(tabId));
            }
        } else {
            publishSnapshotPreviews(provider, &snapshot, converter.orientedSource());
        }
    }

    return snapshot;
}

void TabStateService::restore(DisplayConverter &converter, const ConverterTabSnapshot &snapshot)
{
    ConverterState &state = converter.converterState();
    PreviewImageProvider *provider = converter.previewProvider();
    const QString tabId = snapshot.tabId;

    if (!tabId.isEmpty())
        converter.setActiveTabId(tabId);

    converter.rebuildDebounceTimer()->stop();
    state.rebuildPending = false;

    state.project = snapshot.project;
    state.offsetX = snapshot.project.offsetX;
    state.offsetY = snapshot.project.offsetY;
    state.sourceFilePath = snapshot.sourceFilePath;
    state.projectFile = snapshot.projectFileUrl;
    converter.applySessionSnapshot(snapshot.project.session);

    state.sourceImage = snapshot.sourceImage;
    if (state.sourceImage.isNull()) {
        ConverterTabSnapshot hydrated = snapshot;
        hydrateSourceImage(&hydrated);
        state.sourceImage = hydrated.sourceImage;
        if (state.sourceFilePath.isEmpty())
            state.sourceFilePath = hydrated.sourceFilePath;
    }

    const QString sourceSlot = tabId.isEmpty() ? QStringLiteral("source") : TabPreviewSlots::source(tabId);
    const QString previewSlot = tabId.isEmpty() ? QStringLiteral("preview") : TabPreviewSlots::preview(tabId);
    const QString processSlot = tabId.isEmpty() ? QStringLiteral("process") : TabPreviewSlots::process(tabId);

    if (!state.sourceImage.isNull()) {
        converter.markOrientedDirty();
        if (provider && provider->hasSlot(sourceSlot)) {
            state.sourcePath = provider->imageUrl(sourceSlot);
        } else {
            converter.refreshSourcePreview();
        }
        emit converter.hasImageChanged();
        emit converter.sourceWidthChanged();
        emit converter.sourceHeightChanged();
        emit converter.sourcePathChanged();
    } else {
        state.sourcePath.clear();
        emit converter.hasImageChanged();
        emit converter.sourcePathChanged();
        emit converter.sourceWidthChanged();
        emit converter.sourceHeightChanged();
    }

    if (snapshot.hasPipelineResult && !snapshot.lastResult.preview.isNull()) {
        state.lastResult = snapshot.lastResult;
        state.generatedCode = snapshot.generatedCode;
        state.processPreviewPath = resolvePreviewUrl(provider,
                                                     processSlot,
                                                     state.lastResult.processPreview);
        state.previewPath = resolvePreviewUrl(provider,
                                              previewSlot,
                                              state.lastResult.preview);
        emit converter.processPreviewPathChanged();
        emit converter.previewPathChanged();
        emit converter.generatedCodeChanged();
        ImagePipelineController::updateCodePreview(state, converter);
        ImagePipelineController::updateFlashReport(state, converter);
    } else if (!state.sourceImage.isNull()) {
        const bool hasCachedPreviews = provider
            && provider->hasSlot(previewSlot)
            && provider->hasSlot(processSlot);
        if (hasCachedPreviews) {
            state.previewPath = provider->imageUrl(previewSlot);
            state.processPreviewPath = provider->imageUrl(processSlot);
            emit converter.previewPathChanged();
            emit converter.processPreviewPathChanged();
            ImagePipelineController::updateCodePreview(state, converter);
            ImagePipelineController::updateFlashReport(state, converter);
        } else {
            ImagePipelineController::scheduleRebuild(converter, true);
        }
    } else {
        state.previewPath.clear();
        state.processPreviewPath.clear();
        state.generatedCode.clear();
        state.lastResult = {};
        emit converter.previewPathChanged();
        emit converter.processPreviewPathChanged();
        emit converter.generatedCodeChanged();
        ImagePipelineController::updateCodePreview(state, converter);
        ImagePipelineController::updateFlashReport(state, converter);
    }

    converter.imageTransform()->notifyOffsetChanged();
    converter.updateWatchExportPrefix();
    converter.project()->notifyProjectChanged();
}
