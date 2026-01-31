#include <QtCore/QFile>
#include <QtCore/QLocale>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>

#include "desktopentry.h"
#include "stringconstants.h"

static const auto kDesktopEntry = QStringLiteral("Desktop Entry");
static const auto kExec = QStringLiteral("Exec");
static const auto kIcon = QStringLiteral("Icon");
static const auto kStartupWMClass = QStringLiteral("StartupWMClass");
static const auto kCategories = QStringLiteral("Categories");
static const auto kMimeType = QStringLiteral("MimeType");
static const auto kNoDisplay = QStringLiteral("NoDisplay");
static const auto kXGnomeNoDisplay = QStringLiteral("X-GNOME-NoDisplay");
static const auto kHidden = QStringLiteral("Hidden");
static const auto kTrue = QStringLiteral("true");
static const auto kComment = QStringLiteral("Comment");
static const auto kSuffixExe = QStringLiteral(".exe");
static const auto kSuffixApp = QStringLiteral(".app");
static const auto kExeChrome = QStringLiteral("chrome");
static const auto kExeFirefox = QStringLiteral("firefox");
static const auto kExeMsedge = QStringLiteral("msedge");
static const auto kExeBrave = QStringLiteral("brave");
static const auto kExeOpera = QStringLiteral("opera");
static const auto kExeIexplore = QStringLiteral("iexplore");
static const auto kExeChromium = QStringLiteral("chromium");
static const auto kDisplayGoogleChrome = QStringLiteral("Google Chrome");
static const auto kDisplayMozillaFirefox = QStringLiteral("Mozilla Firefox");
static const auto kDisplayMicrosoftEdge = QStringLiteral("Microsoft Edge");
static const auto kDisplayBrave = QStringLiteral("Brave");
static const auto kDisplayOpera = QStringLiteral("Opera");
static const auto kDisplayInternetExplorer = QStringLiteral("Internet Explorer");
static const auto kDisplayChromium = QStringLiteral("Chromium");

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
        // Skip empty lines and comments.
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        // Check for group header.
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            currentGroup = line.mid(1, line.length() - 2).trimmed();
            inDesktopEntry = (currentGroup.compare(kDesktopEntry, Qt::CaseInsensitive) == 0);
            continue;
        }
        // Parse key=value (only in [Desktop Entry], or when key is a main-entry key like NoDisplay
        // that some files put after [Desktop Action] sections).
        auto equalsPos = line.indexOf(QLatin1Char('='));
        if (equalsPos > 0) {
            auto key = line.left(equalsPos).trimmed();
            auto value = line.mid(equalsPos + 1);
            const bool isMainEntryKey = key.compare(kNoDisplay, Qt::CaseInsensitive) == 0 ||
                                        key.compare(kXGnomeNoDisplay, Qt::CaseInsensitive) == 0 ||
                                        key.compare(kHidden, Qt::CaseInsensitive) == 0;
            if (inDesktopEntry || isMainEntryKey) {
                entries_[key] = value;
            }
        }
    }
    file.close();
    // Extract commonly used values.
    exec_ = getValue(kExec);
    icon_ = getValue(kIcon);
    startupWMClass_ = getValue(kStartupWMClass);
    categories_ = getListValue(kCategories);
    mimeTypes_ = getListValue(kMimeType);
    auto noDisplayVal = getValue(kNoDisplay);
    if (noDisplayVal.isEmpty()) {
        noDisplayVal = getValue(kXGnomeNoDisplay);
    }
    if (noDisplayVal.isEmpty()) {
        noDisplayVal = getValue(kHidden);
    }
    if (noDisplayVal.isEmpty()) {
        for (auto it = entries_.keyBegin(); it != entries_.keyEnd(); ++it) {
            if (it->compare(kNoDisplay, Qt::CaseInsensitive) == 0) {
                noDisplayVal = entries_.value(*it);
                break;
            }
        }
    }
    noDisplay_ = noDisplayVal.trimmed().toLower() == kTrue;
    valid_ = !getValue(kName).isEmpty() && !exec_.isEmpty();
    return valid_;
}

QString DesktopEntry::getValue(const QString &key) const {
    return entries_.value(key, QString());
}

QString DesktopEntry::getLocalizedValue(const QString &key) const {
    // Get the current locale.
    QLocale locale;
    auto language = locale.name(); // e.g., "en_US", "ja_JP"

    // Try full locale (e.g., Name[en_US]).
    static const auto kFmtKeyLocale = QStringLiteral("%1[%2]");
    if (auto value = getValue(kFmtKeyLocale.arg(key, language)); !value.isEmpty()) {
        return value;
    }

    // Try language only (e.g., Name[en]).
    auto langOnly = language.split(QLatin1Char('_')).first();
    if (auto value = getValue(kFmtKeyLocale.arg(key, langOnly)); !value.isEmpty()) {
        return value;
    }

    // Fall back to default (e.g., Name).
    return getValue(key);
}

QString DesktopEntry::name() const {
    return getLocalizedValue(kName);
}

QString DesktopEntry::comment() const {
    return getLocalizedValue(kComment);
}

QStringList DesktopEntry::getListValue(const QString &key) const {
    auto value = getValue(key);
    if (value.isEmpty()) {
        return {};
    }
    // Desktop files use semicolon as separator.
    return value.split(QLatin1Char(';'), Qt::SkipEmptyParts);
}

QString DesktopEntry::executableName() const {
    if (exec_.isEmpty()) {
        return {};
    }
#ifdef Q_OS_MAC
    if (exec_.startsWith(QLatin1Char('/'))) {
        return exec_.mid(exec_.lastIndexOf(QLatin1Char('/')) + 1);
    }
#endif
#ifdef Q_OS_WIN
    const auto lastSlash = exec_.lastIndexOf(QLatin1Char('\\'));
    if (lastSlash >= 0) {
        return exec_.mid(lastSlash + 1);
    }
#endif
    QString current;
    auto inQuote = false;
    QChar quoteChar;
    for (auto i = 0; i < exec_.length(); ++i) {
        auto c = exec_[i];
        if (!inQuote && (c == QLatin1Char('"') || c == QLatin1Char('\''))) {
            inQuote = true;
            quoteChar = c;
        } else if (inQuote && c == quoteChar) {
            inQuote = false;
        } else if (!inQuote && (c == QLatin1Char(' ') || c == QLatin1Char('%'))) {
            if (!current.isEmpty()) {
                return current;
            }
            if (c == QLatin1Char('%')) {
                return {};
            }
        } else {
            current.append(c);
        }
    }
    return current;
}

#ifdef Q_OS_WIN
#include <QtCore/QFileInfo>

bool DesktopEntry::parseFromExecutable(const QString &exePath) {
    valid_ = false;
    filename_ = exePath;
    entries_.clear();
    if (!QFile::exists(exePath) || !exePath.endsWith(kSuffixExe, Qt::CaseInsensitive)) {
        return false;
    }
    exec_ = exePath;
    const auto baseName = QFileInfo(exePath).completeBaseName();
    QString name;
    if (baseName.compare(kExeChrome, Qt::CaseInsensitive) == 0) {
        name = kDisplayGoogleChrome;
    } else if (baseName.compare(kExeFirefox, Qt::CaseInsensitive) == 0) {
        name = kDisplayMozillaFirefox;
    } else if (baseName.compare(kExeMsedge, Qt::CaseInsensitive) == 0) {
        name = kDisplayMicrosoftEdge;
    } else if (baseName.compare(kExeBrave, Qt::CaseInsensitive) == 0) {
        name = kDisplayBrave;
    } else if (baseName.compare(kExeOpera, Qt::CaseInsensitive) == 0) {
        name = kDisplayOpera;
    } else if (baseName.compare(kExeIexplore, Qt::CaseInsensitive) == 0) {
        name = kDisplayInternetExplorer;
    } else if (baseName.compare(kExeChromium, Qt::CaseInsensitive) == 0) {
        name = kDisplayChromium;
    } else {
        name = baseName;
    }
    entries_[kName] = name;
    icon_ = exePath;
    startupWMClass_ = QString();
    categories_ = QStringList();
    mimeTypes_ = QStringList();
    noDisplay_ = false;
    valid_ = true;
    return true;
}
#endif

std::expected<DesktopEntry, DesktopEntryError> readDesktopEntry(const QString &filename) {
#ifdef Q_OS_MAC
    if (filename.endsWith(kSuffixApp)) {
        DesktopEntry entry;
        if (entry.parseAppBundle(filename)) {
            return entry;
        }
        return std::unexpected(DesktopEntryError::ParseFailed);
    }
#elif defined(Q_OS_WIN)
    if (filename.endsWith(kSuffixExe, Qt::CaseInsensitive)) {
        DesktopEntry entry;
        if (entry.parseFromExecutable(filename)) {
            return entry;
        }
        return std::unexpected(DesktopEntryError::ParseFailed);
    }
#endif
    DesktopEntry entry;
    if (entry.parse(filename)) {
        return entry;
    }
    return std::unexpected(DesktopEntryError::ParseFailed);
}
