#include "processing/EncodingAnalyzer.h"
#include "processing/PixelFormatCatalog.h"

#include <limits>
#include <QVariantMap>

QVariantList EncodingAnalyzer::analyze(const DisplayRasterizer::Result &result,
                                       DisplayProfile::ColorMode colorMode,
                                       DisplayCodeGenerator::MonoLayout monoLayout,
                                       DisplayCodeGenerator::EncodingMode currentMode,
                                       const DisplayCodeGenerator::CodeGenOptions &options)
{
    QVariantList rows;
    int bestIndex = -1;
    int bestSize = std::numeric_limits<int>::max();

    const QVariantList modes = DisplayCodeGenerator::availableEncodings();
    for (const QVariant &entry : modes) {
        const QVariantMap modeMap = entry.toMap();
        const auto mode = static_cast<DisplayCodeGenerator::EncodingMode>(modeMap.value(QStringLiteral("mode")).toInt());
        const bool monoMode = PixelFormatCatalog::isMono(mode);
        if (colorMode == DisplayProfile::Mono1Bit && !monoMode)
            continue;
        if (colorMode == DisplayProfile::Rgb565 && monoMode)
            continue;

        DisplayRasterizer::Result sample = result;
        if (mode == DisplayCodeGenerator::EncodingMode::Indexed8)
            DisplayRasterizer::buildIndexedPalette(sample);

        const int bytes = DisplayCodeGenerator::flashFootprintBytes(mode,
                                                                    sample.width,
                                                                    sample.height,
                                                                    sample.monoBits,
                                                                    sample.monoBuffer,
                                                                    sample.grayscale8,
                                                                    sample.rgb565,
                                                                    sample.rgb888,
                                                                    sample.rgb233,
                                                                    sample.rgb24,
                                                                    monoLayout,
                                                                    options,
                                                                    sample.indexedPalette);

        QVariantMap row = modeMap;
        row.insert(QStringLiteral("bytes"), bytes);
        row.insert(QStringLiteral("current"), mode == currentMode);
        row.insert(QStringLiteral("recommended"), false);
        if (bytes > 0 && bytes < bestSize) {
            bestSize = bytes;
            bestIndex = rows.size();
        }
        rows.append(row);
    }

    if (bestIndex >= 0) {
        QVariantMap best = rows.at(bestIndex).toMap();
        best.insert(QStringLiteral("recommended"), true);
        rows[bestIndex] = best;
    }
    return rows;
}
