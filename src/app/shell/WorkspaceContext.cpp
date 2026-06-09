#include "app/shell/WorkspaceContext.h"

#include "app/studio/DisplayConverter.h"
#include "app/studio/controllers/export/ExportController.h"
#include "app/studio/controllers/image/ImageFilterController.h"
#include "app/studio/controllers/image/ImageTransformController.h"
#include "app/studio/controllers/output/CodeGenController.h"
#include "app/studio/controllers/output/DisplayOutputController.h"
#include "app/studio/controllers/output/ViewportController.h"
#include "app/studio/controllers/project/ProjectController.h"
#include "app/tabs/StudioTabController.h"
#include "persistence/AppSettings.h"

WorkspaceContext::WorkspaceContext(DisplayConverter *converter,
                                   StudioTabController *tabs,
                                   AppSettings *settings,
                                   QObject *parent)
    : QObject(parent)
    , m_converter(converter)
    , m_tabs(tabs)
    , m_settings(settings)
{
}

QObject *WorkspaceContext::image() const
{
    return m_converter;
}

QObject *WorkspaceContext::filters() const
{
    return m_converter ? m_converter->imageFilters() : nullptr;
}

QObject *WorkspaceContext::transform() const
{
    return m_converter ? m_converter->imageTransform() : nullptr;
}

QObject *WorkspaceContext::output() const
{
    return m_converter ? m_converter->displayOutput() : nullptr;
}

QObject *WorkspaceContext::code() const
{
    return m_converter ? m_converter->codeGen() : nullptr;
}

QObject *WorkspaceContext::viewport() const
{
    return m_converter ? m_converter->viewport() : nullptr;
}

QObject *WorkspaceContext::project() const
{
    return m_converter ? m_converter->project() : nullptr;
}

QObject *WorkspaceContext::exportPanel() const
{
    return m_converter ? m_converter->exportPanel() : nullptr;
}
