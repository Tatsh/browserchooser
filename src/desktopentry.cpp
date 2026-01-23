#include <QtCore/QFile>
#include <QtCore/QLocale>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>

#include "desktopentry.h"

DesktopEntry::DesktopEntry(const QString &filename) {
    parse(filename);
}

bool DesktopEntry::parse(const QString &filename) {
    valid_ = false;
    filename_ = filename;
    entries_.clear();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    QString currentGroup;
    auto inDesktopEntry = false;
    while (!in.atEnd()) {
        auto line = in.readLine().trimmed();
        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        // Check for group header
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            currentGroup = line.mid(1, line.length() - 2);
            inDesktopEntry = (currentGroup == QStringLiteral("Desktop Entry"));
            continue;
        }
        // Only parse Desktop Entry group
        if (!inDesktopEntry) {
            continue;
        }
        // Parse key=value
        auto equalsPos = line.indexOf(QLatin1Char('='));
        if (equalsPos > 0) {
            auto key = line.left(equalsPos).trimmed();
            auto value = line.mid(equalsPos + 1);
            entries_[key] = value;
        }
    }
    file.close();
    // Extract commonly used values
    exec_ = getValue(QStringLiteral("Exec"));
    icon_ = getValue(QStringLiteral("Icon"));
    startupWMClass_ = getValue(QStringLiteral("StartupWMClass"));
    categories_ = getListValue(QStringLiteral("Categories"));
    mimeTypes_ = getListValue(QStringLiteral("MimeType"));
    noDisplay_ = getValue(QStringLiteral("NoDisplay")).toLower() == QStringLiteral("true");
    valid_ = !getValue(QStringLiteral("Name")).isEmpty() && !exec_.isEmpty();
    return valid_;
}

QString DesktopEntry::getValue(const QString &key) const {
    return entries_.value(key, QString());
}

QString DesktopEntry::getLocalizedValue(const QString &key) const {
    // Get the current locale
    QLocale locale;
    auto language = locale.name(); // e.g., "en_US", "ja_JP"

    // Try full locale (e.g., Name[en_US])
    if (auto value = getValue(QStringLiteral("%1[%2]").arg(key, language)); !value.isEmpty()) {
        return value;
    }

    // Try language only (e.g., Name[en])
    auto langOnly = language.split(QLatin1Char('_')).first();
    if (auto value = getValue(QStringLiteral("%1[%2]").arg(key, langOnly)); !value.isEmpty()) {
        return value;
    }

    // Fall back to default (e.g., Name)
    return getValue(key);
}

QString DesktopEntry::name() const {
    return getLocalizedValue(QStringLiteral("Name"));
}

QString DesktopEntry::comment() const {
    return getLocalizedValue(QStringLiteral("Comment"));
}

QStringList DesktopEntry::getListValue(const QString &key) const {
    auto value = getValue(key);
    if (value.isEmpty()) {
        return {};
    }
    // Desktop files use semicolon as separator
    return value.split(QLatin1Char(';'), Qt::SkipEmptyParts);
}

std::expected<DesktopEntry, QString> readDesktopEntry(const QString &filename) {
    DesktopEntry entry;
    if (entry.parse(filename)) {
        return entry;
    }
    return std::unexpected(QStringLiteral("Failed to parse desktop entry: ") + filename);
}
