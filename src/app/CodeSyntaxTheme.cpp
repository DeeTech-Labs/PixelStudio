#include "app/CodeSyntaxTheme.h"

#include <QChar>
#include <QColor>
#include <QRegularExpression>
#include <QSettings>

namespace {

constexpr QLatin1StringView kKeyComment("comment");
constexpr QLatin1StringView kKeyDirective("directive");
constexpr QLatin1StringView kKeyKeyword("keyword");
constexpr QLatin1StringView kKeyType("type");
constexpr QLatin1StringView kKeyNumber("number");
constexpr QLatin1StringView kKeyString("string");
constexpr QLatin1StringView kKeyMacro("macro");
constexpr QLatin1StringView kKeyIdentifier("identifier");
constexpr QLatin1StringView kKeyPunctuation("punctuation");
constexpr QLatin1StringView kKeyText("text");

bool isIdentifierStart(QChar ch)
{
    return ch.isLetter() || ch == QLatin1Char('_');
}

bool isIdentifierPart(QChar ch)
{
    return ch.isLetterOrNumber() || ch == QLatin1Char('_');
}

bool isKeyword(QStringView word)
{
    static const QStringList kKeywords = {
        QStringLiteral("static"), QStringLiteral("const"), QStringLiteral("unsigned"),
        QStringLiteral("signed"), QStringLiteral("void"),   QStringLiteral("char"),
        QStringLiteral("int"),    QStringLiteral("short"),  QStringLiteral("long"),
        QStringLiteral("float"),  QStringLiteral("double"), QStringLiteral("volatile"),
        QStringLiteral("extern"),
    };
    return kKeywords.contains(word);
}

bool isTypeName(QStringView word)
{
    static const QStringList kTypes = {
        QStringLiteral("uint8_t"),  QStringLiteral("uint16_t"), QStringLiteral("uint32_t"),
        QStringLiteral("uint64_t"), QStringLiteral("int8_t"),   QStringLiteral("int16_t"),
        QStringLiteral("int32_t"),  QStringLiteral("int64_t"),  QStringLiteral("size_t"),
        QStringLiteral("PROGMEM"),
    };
    return kTypes.contains(word);
}

bool isMacroName(QStringView word)
{
    if (word.isEmpty())
        return false;
    bool hasUpper = false;
    for (const QChar ch : word) {
        if (ch == QLatin1Char('_'))
            continue;
        if (ch.isUpper())
            hasUpper = true;
        else if (!ch.isDigit())
            return false;
    }
    return hasUpper;
}

QString escapeHtml(QStringView raw)
{
    QString out;
    out.reserve(int(raw.size()) + 8);
    for (const QChar ch : raw) {
        if (ch == QLatin1Char('&')) {
            out += QStringLiteral("&amp;");
        } else if (ch == QLatin1Char('<')) {
            out += QStringLiteral("&lt;");
        } else if (ch == QLatin1Char('>')) {
            out += QStringLiteral("&gt;");
        } else if (ch == QLatin1Char(' ')) {
            out += QStringLiteral("&nbsp;");
        } else if (ch == QLatin1Char('\t')) {
            out += QStringLiteral("&nbsp;&nbsp;&nbsp;&nbsp;");
        } else {
            out += ch;
        }
    }
    return out;
}

void appendSpan(QString &html, QStringView text, const QString &color)
{
    if (text.isEmpty())
        return;
    html += QStringLiteral("<span style=\"color:%1\">%2</span>")
                .arg(color, escapeHtml(text));
}

QString colorOr(const QVariantMap &colors, QLatin1StringView key, const QString &fallback)
{
    return colors.value(key).toString().isEmpty() ? fallback : colors.value(key).toString();
}

int skipSpaces(QStringView line, int index)
{
    while (index < line.size() && line.at(index).isSpace())
        ++index;
    return index;
}

int readIdentifier(QStringView line, int index)
{
    while (index < line.size() && isIdentifierPart(line.at(index)))
        ++index;
    return index;
}

bool isHexDigit(QChar ch)
{
    return ch.isDigit()
        || ch == QLatin1Char('a') || ch == QLatin1Char('A')
        || ch == QLatin1Char('b') || ch == QLatin1Char('B')
        || ch == QLatin1Char('c') || ch == QLatin1Char('C')
        || ch == QLatin1Char('d') || ch == QLatin1Char('D')
        || ch == QLatin1Char('e') || ch == QLatin1Char('E')
        || ch == QLatin1Char('f') || ch == QLatin1Char('F');
}

int readNumber(QStringView line, int index)
{
    if (index + 1 < line.size() && line.at(index) == QLatin1Char('0')
        && (line.at(index + 1) == QLatin1Char('x') || line.at(index + 1) == QLatin1Char('X'))) {
        index += 2;
        while (index < line.size() && isHexDigit(line.at(index)))
            ++index;
        return index;
    }

    while (index < line.size() && line.at(index).isDigit())
        ++index;
    if (index < line.size() && line.at(index) == QLatin1Char('.')) {
        ++index;
        while (index < line.size() && line.at(index).isDigit())
            ++index;
    }
    return index;
}

int readString(QStringView line, int index, QChar quote)
{
    ++index;
    while (index < line.size()) {
        if (line.at(index) == QLatin1Char('\\') && index + 1 < line.size()) {
            index += 2;
            continue;
        }
        if (line.at(index) == quote)
            return index + 1;
        ++index;
    }
    return line.size();
}

void highlightSpaces(QString &html, QStringView line, int from, int to, const QVariantMap &colors)
{
    if (from < to)
        appendSpan(html, line.mid(from, to - from), colorOr(colors, kKeyText, QStringLiteral("#D4D4D4")));
}

void highlightPreprocessorLine(QString &html, QStringView line, const QVariantMap &colors)
{
    int index = 0;
    const int spacesEnd = skipSpaces(line, index);
    highlightSpaces(html, line, index, spacesEnd, colors);
    index = spacesEnd;

    if (index < line.size() && line.at(index) == QLatin1Char('#')) {
        appendSpan(html, QStringView(line).mid(index, 1),
                   colorOr(colors, kKeyPunctuation, QStringLiteral("#D4D4D4")));
        ++index;
    }

    index = skipSpaces(line, index);
    const int directiveStart = index;
    index = readIdentifier(line, index);
    const QString directive = line.mid(directiveStart, index - directiveStart).toString();
    appendSpan(html, directive, colorOr(colors, kKeyDirective, QStringLiteral("#C586C0")));

    bool expectMacro = directive == QStringLiteral("define");
    while (index < line.size()) {
        const int spacesStart = index;
        index = skipSpaces(line, index);
        highlightSpaces(html, line, spacesStart, index, colors);
        if (index >= line.size())
            break;

        const QChar ch = line.at(index);
        if (ch == QLatin1Char('"') || ch == QLatin1Char('\'')) {
            const int start = index;
            index = readString(line, index, ch);
            appendSpan(html, line.mid(start, index - start),
                       colorOr(colors, kKeyString, QStringLiteral("#CE9178")));
            continue;
        }
        if (ch.isDigit()
            || (ch == QLatin1Char('0') && index + 1 < line.size()
                && (line.at(index + 1) == QLatin1Char('x') || line.at(index + 1) == QLatin1Char('X')))) {
            const int start = index;
            index = readNumber(line, index);
            appendSpan(html, line.mid(start, index - start),
                       colorOr(colors, kKeyNumber, QStringLiteral("#B5CEA8")));
            continue;
        }
        if (isIdentifierStart(ch)) {
            const int start = index;
            index = readIdentifier(line, index);
            const QStringView word = line.mid(start, index - start);
            if (expectMacro) {
                appendSpan(html, word, colorOr(colors, kKeyMacro, QStringLiteral("#4FC1FF")));
                expectMacro = false;
            } else if (isMacroName(word)) {
                appendSpan(html, word, colorOr(colors, kKeyMacro, QStringLiteral("#4FC1FF")));
            } else if (isKeyword(word)) {
                appendSpan(html, word, colorOr(colors, kKeyKeyword, QStringLiteral("#569CD6")));
            } else if (isTypeName(word)) {
                appendSpan(html, word, colorOr(colors, kKeyType, QStringLiteral("#4EC9B0")));
            } else {
                appendSpan(html, word, colorOr(colors, kKeyIdentifier, QStringLiteral("#9CDCFE")));
            }
            continue;
        }

        appendSpan(html, QStringView(line).mid(index, 1),
                   colorOr(colors, kKeyPunctuation, QStringLiteral("#D4D4D4")));
        ++index;
    }
}

void highlightCodeLine(QString &html, QStringView line, const QVariantMap &colors)
{
    int index = 0;
    while (index < line.size()) {
        const int spacesStart = index;
        index = skipSpaces(line, index);
        highlightSpaces(html, line, spacesStart, index, colors);
        if (index >= line.size())
            break;

        const QChar ch = line.at(index);
        if (ch == QLatin1Char('/') && index + 1 < line.size() && line.at(index + 1) == QLatin1Char('/')) {
            appendSpan(html, line.mid(index), colorOr(colors, kKeyComment, QStringLiteral("#6A9955")));
            break;
        }
        if (ch == QLatin1Char('"') || ch == QLatin1Char('\'')) {
            const int start = index;
            index = readString(line, index, ch);
            appendSpan(html, line.mid(start, index - start),
                       colorOr(colors, kKeyString, QStringLiteral("#CE9178")));
            continue;
        }
        if (ch.isDigit()
            || (ch == QLatin1Char('0') && index + 1 < line.size()
                && (line.at(index + 1) == QLatin1Char('x') || line.at(index + 1) == QLatin1Char('X')))) {
            const int start = index;
            index = readNumber(line, index);
            appendSpan(html, line.mid(start, index - start),
                       colorOr(colors, kKeyNumber, QStringLiteral("#B5CEA8")));
            continue;
        }
        if (isIdentifierStart(ch)) {
            const int start = index;
            index = readIdentifier(line, index);
            const QStringView word = line.mid(start, index - start);
            if (isKeyword(word)) {
                appendSpan(html, word, colorOr(colors, kKeyKeyword, QStringLiteral("#569CD6")));
            } else if (isTypeName(word)) {
                appendSpan(html, word, colorOr(colors, kKeyType, QStringLiteral("#4EC9B0")));
            } else if (isMacroName(word)) {
                appendSpan(html, word, colorOr(colors, kKeyMacro, QStringLiteral("#4FC1FF")));
            } else {
                appendSpan(html, word, colorOr(colors, kKeyIdentifier, QStringLiteral("#9CDCFE")));
            }
            continue;
        }

        appendSpan(html, QStringView(line).mid(index, 1),
                   colorOr(colors, kKeyPunctuation, QStringLiteral("#D4D4D4")));
        ++index;
    }
}

} // namespace

CodeSyntaxTheme::CodeSyntaxTheme(QObject *parent)
    : QObject(parent)
{
    applyDefaults(this);
}

void CodeSyntaxTheme::applyDefaults(CodeSyntaxTheme *theme)
{
    theme->m_comment = QStringLiteral("#6A9955");
    theme->m_directive = QStringLiteral("#C586C0");
    theme->m_keyword = QStringLiteral("#569CD6");
    theme->m_type = QStringLiteral("#4EC9B0");
    theme->m_number = QStringLiteral("#B5CEA8");
    theme->m_string = QStringLiteral("#CE9178");
    theme->m_macro = QStringLiteral("#4FC1FF");
    theme->m_identifier = QStringLiteral("#9CDCFE");
    theme->m_punctuation = QStringLiteral("#D4D4D4");
    theme->m_text = QStringLiteral("#D4D4D4");
}

QString CodeSyntaxTheme::normalizeColor(const QString &value, const QString &fallback)
{
    const QString trimmed = value.trimmed();
    if (trimmed.isEmpty())
        return fallback;

    QColor color(trimmed);
    if (!color.isValid()) {
        const QRegularExpression hex(QStringLiteral("^#?[0-9A-Fa-f]{6}$"));
        if (hex.match(trimmed).hasMatch()) {
            QString normalized = trimmed;
            if (!normalized.startsWith(QLatin1Char('#')))
                normalized.prepend(QLatin1Char('#'));
            color = QColor(normalized);
        }
    }
    return color.isValid() ? color.name(QColor::HexRgb) : fallback;
}

void CodeSyntaxTheme::load(QSettings &settings)
{
    m_settings = &settings;
    m_comment = normalizeColor(settings.value(QStringLiteral("codeSyntax/comment")).toString(), m_comment);
    m_directive = normalizeColor(settings.value(QStringLiteral("codeSyntax/directive")).toString(), m_directive);
    m_keyword = normalizeColor(settings.value(QStringLiteral("codeSyntax/keyword")).toString(), m_keyword);
    m_type = normalizeColor(settings.value(QStringLiteral("codeSyntax/type")).toString(), m_type);
    m_number = normalizeColor(settings.value(QStringLiteral("codeSyntax/number")).toString(), m_number);
    m_string = normalizeColor(settings.value(QStringLiteral("codeSyntax/string")).toString(), m_string);
    m_macro = normalizeColor(settings.value(QStringLiteral("codeSyntax/macro")).toString(), m_macro);
    m_identifier = normalizeColor(settings.value(QStringLiteral("codeSyntax/identifier")).toString(), m_identifier);
    m_punctuation = normalizeColor(settings.value(QStringLiteral("codeSyntax/punctuation")).toString(), m_punctuation);
    m_text = normalizeColor(settings.value(QStringLiteral("codeSyntax/text")).toString(), m_text);
    ++m_revision;
    emit changed();
}

QVariantMap CodeSyntaxTheme::asMap() const
{
    return {
        {QString::fromLatin1(kKeyComment), m_comment},
        {QString::fromLatin1(kKeyDirective), m_directive},
        {QString::fromLatin1(kKeyKeyword), m_keyword},
        {QString::fromLatin1(kKeyType), m_type},
        {QString::fromLatin1(kKeyNumber), m_number},
        {QString::fromLatin1(kKeyString), m_string},
        {QString::fromLatin1(kKeyMacro), m_macro},
        {QString::fromLatin1(kKeyIdentifier), m_identifier},
        {QString::fromLatin1(kKeyPunctuation), m_punctuation},
        {QString::fromLatin1(kKeyText), m_text},
    };
}

QString CodeSyntaxTheme::highlight(const QString &code) const
{
    if (code.isEmpty())
        return QString();

    const QVariantMap colors = asMap();
    QString html;
    html.reserve(code.size() * 2);

    const QStringList lines = code.split(QLatin1Char('\n'));
    for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        const QString &line = lines.at(lineIndex);
        const QStringView view(line);
        const int firstNonSpace = skipSpaces(view, 0);
        if (firstNonSpace < view.size() && view.at(firstNonSpace) == QLatin1Char('#'))
            highlightPreprocessorLine(html, view, colors);
        else if (firstNonSpace < view.size() && view.at(firstNonSpace) == QLatin1Char('/')
                 && firstNonSpace + 1 < view.size() && view.at(firstNonSpace + 1) == QLatin1Char('/'))
            appendSpan(html, view, colorOr(colors, kKeyComment, QStringLiteral("#6A9955")));
        else
            highlightCodeLine(html, view, colors);

        if (lineIndex + 1 < lines.size())
            html += QStringLiteral("<br>");
    }
    return html;
}

void CodeSyntaxTheme::resetDefaults()
{
    applyDefaults(this);
    if (m_settings) {
        m_settings->remove(QStringLiteral("codeSyntax"));
        m_settings->sync();
    }
    ++m_revision;
    emit changed();
}

void CodeSyntaxTheme::setColor(QString &field, const QString &value, const char *settingsKey)
{
    const QString normalized = normalizeColor(value, field);
    if (field == normalized)
        return;
    field = normalized;
    saveColor(settingsKey, field);
    ++m_revision;
    emit changed();
}

void CodeSyntaxTheme::saveColor(const char *settingsKey, const QString &value)
{
    if (!m_settings)
        return;
    m_settings->setValue(QString::fromLatin1(settingsKey), value);
    m_settings->sync();
}

void CodeSyntaxTheme::setComment(const QString &value)
{
    setColor(m_comment, value, "codeSyntax/comment");
}

void CodeSyntaxTheme::setDirective(const QString &value)
{
    setColor(m_directive, value, "codeSyntax/directive");
}

void CodeSyntaxTheme::setKeyword(const QString &value)
{
    setColor(m_keyword, value, "codeSyntax/keyword");
}

void CodeSyntaxTheme::setType(const QString &value)
{
    setColor(m_type, value, "codeSyntax/type");
}

void CodeSyntaxTheme::setNumber(const QString &value)
{
    setColor(m_number, value, "codeSyntax/number");
}

void CodeSyntaxTheme::setString(const QString &value)
{
    setColor(m_string, value, "codeSyntax/string");
}

void CodeSyntaxTheme::setMacro(const QString &value)
{
    setColor(m_macro, value, "codeSyntax/macro");
}

void CodeSyntaxTheme::setIdentifier(const QString &value)
{
    setColor(m_identifier, value, "codeSyntax/identifier");
}

void CodeSyntaxTheme::setPunctuation(const QString &value)
{
    setColor(m_punctuation, value, "codeSyntax/punctuation");
}

void CodeSyntaxTheme::setText(const QString &value)
{
    setColor(m_text, value, "codeSyntax/text");
}
