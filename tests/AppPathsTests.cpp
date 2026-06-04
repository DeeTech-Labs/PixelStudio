#include <QDir>
#include <QTemporaryDir>
#include <QString>

#include "persistence/AppPaths.h"
#include "persistence/StoredPath.h"

#include <iostream>

namespace {

int fail(const char *message)
{
    std::cerr << "FAIL: " << message << '\n';
    return 1;
}

} // namespace

int main()
{
    QTemporaryDir dataDir;
    QTemporaryDir docsDir;
    if (!dataDir.isValid() || !docsDir.isValid())
        return fail("temporary directories");

    AppPaths::setTestRoots(dataDir.path(), docsDir.path());
    AppPaths::ensureLayout();

    if (!QDir(AppPaths::projectsDir()).exists())
        return fail("projects directory should exist");
    if (!QDir(AppPaths::exportsDir()).exists())
        return fail("exports directory should exist");
    if (!QDir(AppPaths::watchDir()).exists())
        return fail("watch directory should exist");

    const QString inside = docsDir.path() + QStringLiteral("/projects/demo.png");
    QDir().mkpath(QFileInfo(inside).absolutePath());
    QFile file(inside);
    if (!file.open(QIODevice::WriteOnly))
        return fail("create demo file");
    file.write("x");
    file.close();

    const QString encoded = StoredPath::encode(inside);
    const QString decoded = StoredPath::decode(encoded);
    if (decoded != QDir::cleanPath(inside))
        return fail("StoredPath round-trip inside documents root");

    AppPaths::clearTestRoots();
    std::cout << "OK: AppPathsTests passed\n";
    return 0;
}
