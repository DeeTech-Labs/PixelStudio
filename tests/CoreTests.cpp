#include <QByteArray>
#include <QImage>
#include <QString>

#include "processing/DisplayCodeGenerator.h"
#include "processing/EncodingAnalyzer.h"
#include "processing/DisplayProfile.h"
#include "processing/DisplayRasterizer.h"
#include "persistence/ProjectFormat.h"
#include "persistence/ProjectService.h"

#include <iostream>

namespace {

int fail(const char *message)
{
    std::cerr << "FAIL: " << message << '\n';
    return 1;
}

DisplayProfile monoProfile()
{
    DisplayProfile p;
    p.id = QStringLiteral("128x64");
    p.name = QStringLiteral("128×64");
    p.colorMode = DisplayProfile::Mono1Bit;
    return p;
}

DisplayProfile rgbProfile()
{
    DisplayProfile p;
    p.id = QStringLiteral("240x240");
    p.name = QStringLiteral("240×240");
    p.colorMode = DisplayProfile::Rgb565;
    return p;
}

} // namespace

int main()
{
    if (DisplayCodeGenerator::sanitizeIdentifier(QStringLiteral("  12 bad-name  "))
        != QStringLiteral("_badname")) {
        return fail("sanitizeIdentifier should normalize invalid prefix and symbols");
    }

    QVector<bool> monoBits(8);
    for (int i = 0; i < monoBits.size(); ++i)
        monoBits[i] = (i % 2) == 0;
    const QString rowCode = DisplayCodeGenerator::generate(monoProfile(),
                                                            8,
                                                            1,
                                                            DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb,
                                                            monoBits,
                                                            QByteArray(),
                                                            QByteArray(),
                                                            {},
                                                            QByteArray(),
                                                            QByteArray(),
                                                            {},
                                                            QStringLiteral("icon"),
                                                            DisplayCodeGenerator::MonoLayout::RowPacked);
    if (!rowCode.contains(QStringLiteral("0xaa"), Qt::CaseInsensitive))
        return fail("row-packed output should contain 0xAA for alternating bits");

    QByteArray pageBuf;
    pageBuf.append(char(0x55));
    pageBuf.append(char(0x0f));
    const QVector<bool> pageBits(16, false);
    const QString pageCode = DisplayCodeGenerator::generate(monoProfile(),
                                                             2,
                                                             8,
                                                             DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb,
                                                             pageBits,
                                                             pageBuf,
                                                             QByteArray(),
                                                             {},
                                                             QByteArray(),
                                                             QByteArray(),
                                                             {},
                                                             QStringLiteral("icon"),
                                                             DisplayCodeGenerator::MonoLayout::Ssd1306Page);
    if (!pageCode.contains(QStringLiteral("0x55, 0x0f"), Qt::CaseInsensitive))
        return fail("Page layout should use provided page buffer bytes");

    if (pageCode.contains(QStringLiteral("// Driver:")))
        return fail("Generated code should not include driver comments");

    QVector<quint16> rgb565(1);
    rgb565[0] = 0xf800;
    const QString rgbCode = DisplayCodeGenerator::generate(rgbProfile(),
                                                            1,
                                                            1,
                                                            DisplayCodeGenerator::EncodingMode::Rgb565,
                                                            {},
                                                            {},
                                                            QByteArray(),
                                                            rgb565,
                                                            QByteArray(),
                                                            QByteArray(),
                                                            {},
                                                            QStringLiteral("pixel"));
    if (!rgbCode.contains(QStringLiteral("0xf800"), Qt::CaseInsensitive))
        return fail("RGB565 code should contain encoded pixel value");

    const QByteArray emptyMonoPacked = DisplayCodeGenerator::binaryData(
        DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb,
        128,
        64,
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        DisplayCodeGenerator::MonoLayout::RowPacked);
    if (emptyMonoPacked.size() != 1024)
        return fail("binaryData without pixels should return expected mono size, not crash");

    const QByteArray monoPacked = DisplayCodeGenerator::binaryData(
        DisplayCodeGenerator::EncodingMode::Mono8HorizontalLsb,
        8,
        1,
        monoBits,
        {},
        {},
        {},
        {},
        {},
        {},
        DisplayCodeGenerator::MonoLayout::RowPacked);
    if (monoPacked.size() != 1 || quint8(monoPacked.at(0)) != 0x55)
        return fail("Mono8HorizontalLsb should pack bits as 0x55");

    const QByteArray gyverBitmap = DisplayCodeGenerator::binaryData(
        DisplayCodeGenerator::EncodingMode::PackedImageHeader,
        8,
        1,
        monoBits,
        {},
        {},
        {},
        {},
        {},
        {},
        DisplayCodeGenerator::MonoLayout::RowPacked);
    if (gyverBitmap.size() < 5)
        return fail("PackedImageHeader should contain header + payload");

    QVector<quint32> rgb24(1);
    rgb24[0] = 0x112233;
    const QString rgb24Code = DisplayCodeGenerator::generate(rgbProfile(),
                                                              1,
                                                              1,
                                                              DisplayCodeGenerator::EncodingMode::Rgb24,
                                                              {},
                                                              {},
                                                              QByteArray(),
                                                              {},
                                                              QByteArray(),
                                                              QByteArray(),
                                                              rgb24,
                                                              QStringLiteral("rgb24"));
    if (!rgb24Code.contains(QStringLiteral("0x112233"), Qt::CaseInsensitive))
        return fail("RGB24 code should contain packed color value");

    QImage whitePixel(1, 1, QImage::Format_ARGB32);
    whitePixel.fill(Qt::white);
    ImageFiltersPipeline::Params filters;
    const auto monoResult = DisplayRasterizer::convert(whitePixel,
                                                        1,
                                                        1,
                                                        DisplayProfile::Mono1Bit,
                                                        DisplayProfile::Stretch,
                                                        filters,
                                                        DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb,
                                                        128,
                                                        true);
    if (monoResult.monoBits.isEmpty() || monoResult.monoBits[0])
        return fail("invert mono should flip white pixel to off");
    if (monoResult.monoBuffer.size() != 1 || monoResult.monoBuffer.at(0) != 0x00)
        return fail("invert mono should regenerate page buffer");

    const QVariantList report = EncodingAnalyzer::analyze(monoResult,
                                                          DisplayProfile::Mono1Bit,
                                                          DisplayCodeGenerator::MonoLayout::RowPacked,
                                                          DisplayCodeGenerator::EncodingMode::Mono8HorizontalMsb);
    if (report.isEmpty())
        return fail("Encoding analyzer should produce flash report");

    StudioProject project = ProjectService::fromSession(QStringLiteral("Unit"),
                                                          SessionSnapshot{},
                                                          3,
                                                          4);
    if (project.offsetX != 3 || project.offsetY != 4)
        return fail("ProjectService should preserve project metadata");

    const QByteArray encoded = ProjectFormat::encode(project);
    StudioProject decoded;
    if (!ProjectFormat::decode(encoded, &decoded, nullptr))
        return fail("ProjectFormat round-trip should succeed");
    if (decoded.name != project.name || decoded.offsetX != project.offsetX)
        return fail("ProjectFormat round-trip should preserve fields");

    return 0;
}
