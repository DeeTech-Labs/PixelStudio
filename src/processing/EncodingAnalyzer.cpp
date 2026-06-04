#include "processing/EncodingAnalyzer.h"

#include <limits>
#include <QVariantMap>

QVariantList EncodingAnalyzer::analyze(const DisplayRasterizer::Result &result,
                                       DisplayProfile::ColorMode colorMode,
                                       DisplayCodeGenerator::MonoLayout monoLayout,
                                       DisplayCodeGenerator::EncodingMode currentMode)
{
    QVariantList rows;
    int bestIndex = -1;
    int bestSize = std::numeric_limits<int>::max();

    const QVariantList modes = DisplayCodeGenerator::availableEncodings();
    for (const QVariant &entry : modes) {
        const QVariantMap modeMap = entry.toMap();
        const auto mode = static_cast<DisplayCodeGenerator::EncodingMode>(modeMap.value(QStringLiteral("mode")).toInt());
        const bool monoMode = static_cast<int>(mode) <= static_cast<int>(DisplayCodeGenerator::EncodingMode::PackedImageRle)
            || mode == DisplayCodeGenerator::EncodingMode::Ascii
            || mode == DisplayCodeGenerator::EncodingMode::Bricks;
        if (colorMode == DisplayProfile::Mono1Bit && !monoMode)
            continue;
        if (colorMode == DisplayProfile::Rgb565 && monoMode)
            continue;

        const QByteArray data = DisplayCodeGenerator::binaryData(mode,
                                                                 result.width,
                                                                 result.height,
                                                                 result.monoBits,
                                                                 result.monoBuffer,
                                                                 result.grayscale8,
                                                                 result.rgb565,
                                                                 result.rgb888,
                                                                 result.rgb233,
                                                                 result.rgb24,
                                                                 monoLayout);
        QVariantMap row = modeMap;
        row.insert(QStringLiteral("bytes"), data.size());
        row.insert(QStringLiteral("current"), mode == currentMode);
        row.insert(QStringLiteral("recommended"), false);
        if (data.size() > 0 && data.size() < bestSize) {
            bestSize = data.size();
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
