#include "app/WinTaskbarRecent.h"

#include "translation/AppLocale.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#endif

namespace {

#ifdef Q_OS_WIN
constexpr wchar_t kAppUserModelId[] = L"DeeTech.PixelStudio";

bool setShellLinkTitle(IShellLinkW *link, const QString &title)
{
    if (!link)
        return false;
    IPropertyStore *store = nullptr;
    if (FAILED(link->QueryInterface(IID_PPV_ARGS(&store))))
        return false;

    PROPVARIANT prop;
    PropVariantInit(&prop);
    const std::wstring wtitle = title.toStdWString();
    const bool ok = SUCCEEDED(InitPropVariantFromString(wtitle.c_str(), &prop));
    if (ok)
        store->SetValue(PKEY_Title, prop);
    PropVariantClear(&prop);
    if (ok)
        store->Commit();
    store->Release();
    return ok;
}

IShellLinkW *shellLinkForRecentFile(const QString &filePath, const QString &title)
{
    const QString exePath = QCoreApplication::applicationFilePath();
    if (exePath.isEmpty() || filePath.isEmpty())
        return nullptr;

    IShellLinkW *link = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link))))
        return nullptr;

    const std::wstring wexe = QDir::toNativeSeparators(exePath).toStdWString();
    const std::wstring wfile = QDir::toNativeSeparators(filePath).toStdWString();
    const std::wstring wdir = QDir::toNativeSeparators(QFileInfo(filePath).absolutePath()).toStdWString();
    const std::wstring wargs = L"\"" + wfile + L"\"";

    link->SetPath(wexe.c_str());
    link->SetArguments(wargs.c_str());
    if (!wdir.empty())
        link->SetWorkingDirectory(wdir.c_str());
    setShellLinkTitle(link, title);
    link->SetDescription(title.toStdWString().c_str());

    return link;
}
#endif

} // namespace

void WinTaskbarRecent::installIdentity()
{
#ifdef Q_OS_WIN
    SetCurrentProcessExplicitAppUserModelID(kAppUserModelId);
#endif
}

WinTaskbarRecent::WinTaskbarRecent(QObject *parent)
    : QObject(parent)
{
}

void WinTaskbarRecent::syncFromRecentFiles(const QVariantList &recent)
{
#ifdef Q_OS_WIN
    ICustomDestinationList *destList = nullptr;
    if (FAILED(CoCreateInstance(CLSID_DestinationList, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&destList))))
        return;

    destList->SetAppID(kAppUserModelId);

    UINT minSlots = 0;
    IObjectArray *removed = nullptr;
    if (FAILED(destList->BeginList(&minSlots, IID_PPV_ARGS(&removed)))) {
        destList->Release();
        return;
    }
    if (removed)
        removed->Release();

    IObjectCollection *collection = nullptr;
    if (FAILED(CoCreateInstance(CLSID_EnumerableObjectCollection, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&collection)))) {
        destList->AbortList();
        destList->Release();
        return;
    }

    QStringList seen;
    int added = 0;
    constexpr int kMaxJumpItems = 10;
    for (const QVariant &v : recent) {
        if (added >= kMaxJumpItems)
            break;
        const QVariantMap row = v.toMap();
        const QString path = row.value(QStringLiteral("path")).toString();
        if (path.isEmpty() || !QFileInfo::exists(path))
            continue;
        const QString canonical = QFileInfo(path).canonicalFilePath();
        const QString key = canonical.isEmpty() ? path : canonical;
        if (seen.contains(key))
            continue;
        seen.append(key);

        const QString title = row.value(QStringLiteral("name"), QFileInfo(path).fileName()).toString();
        IShellLinkW *link = shellLinkForRecentFile(path, title);
        if (!link)
            continue;

        if (SUCCEEDED(collection->AddObject(link))) {
            link->Release();
            ++added;
        } else {
            link->Release();
        }
    }

    if (added > 0) {
        const std::wstring category = AppLocale::tr("Recent").toStdWString();
        destList->AppendCategory(category.c_str(), collection);
    }
    collection->Release();

    destList->CommitList();
    destList->Release();
#else
    Q_UNUSED(recent);
#endif
}
