#include <ranges>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonValue>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "backend.h"
#include "chromeprofile.h"
#include "desktopentry.h"
#include "stringconstants.h"

namespace {

QString *configFilePathOverride() {
    static QString overridePath;
    return &overridePath;
}

} // anonymous namespace

QString getConfigFilePath() {
    const auto *override = configFilePathOverride();
    if (!override->isEmpty()) {
        return *override;
    }
    // LCOV_EXCL_START
    static const auto kFmt = QStringLiteral("%1/browserchooserrc");
    const auto configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return kFmt.arg(configDir);
    // LCOV_EXCL_STOP
}

void setConfigFilePathOverride(const QString &path) {
    *configFilePathOverride() = path;
}

void clearConfigFilePathOverride() {
    configFilePathOverride()->clear();
}

QStringList readCommaSeparatedList(const QString &key) {
    if (key.isEmpty()) {
        return {}; // LCOV_EXCL_LINE
    }
    const auto raw = QSettings(getConfigFilePath(), QSettings::IniFormat).value(key).toString();
    auto list = raw.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (auto &s : list) {
        s = s.trimmed();
    }
    list.removeAll(QString());
    return list;
}

bool listContainsIdentifier(const QStringList &list, const QString &identifier) {
    if (identifier.isEmpty()) {
        return false;
    }
    return std::ranges::any_of(list, [&identifier](const QString &s) {
        return s.compare(identifier, Qt::CaseInsensitive) == 0;
    });
}

bool isBrowserHidden(const BrowserOption &option, const QStringList &hiddenIdentifiers) {
    const auto baseName = QFileInfo(option.entry().filename()).completeBaseName();
    return listContainsIdentifier(hiddenIdentifiers, baseName);
}

#ifndef Q_OS_WIN
// POSIX: single-quote style per Python shlex.quote.
// https://github.com/python/cpython/blob/3.14/Lib/shlex.py#L320
static const auto kEmptyQuoted = QStringLiteral("''");
static const auto kSingleQuoteEscape = QStringLiteral("'\"'\"'");

static bool isSafeShellChar(QChar c) {
    const ushort u = c.unicode();
    if (u >= 128) {
        return false;
    }
    if (u >= '0' && u <= '9') {
        return true;
    }
    if (u >= 'A' && u <= 'Z') {
        return true;
    }
    if (u >= 'a' && u <= 'z') {
        return true;
    }
    return u == '%' || u == '+' || u == ',' || u == '-' || u == '.' || u == '/' || u == ':' ||
           u == '=' || u == '@' || u == '_';
}

QString quoteArg(const QString &arg) {
    if (arg.isEmpty()) {
        return kEmptyQuoted;
    }
    if (std::ranges::all_of(arg, isSafeShellChar)) {
        return arg;
    }
    QString result;
    result.reserve(arg.size() + 2);
    result += QLatin1Char('\'');
    for (const auto c : arg) {
        if (c == QLatin1Char('\'')) {
            result += kSingleQuoteEscape;
        } else {
            result += c;
        }
    }
    result += QLatin1Char('\'');
    return result;
}
#endif // !Q_OS_WIN

void sortBrowserOptionsByDisplayName(QList<BrowserOption> &options) {
    std::ranges::sort(options, [](const BrowserOption &a, const BrowserOption &b) {
        return a.displayName().compare(b.displayName(), Qt::CaseInsensitive) < 0;
    });
}

static QList<QStringList> readCommandListFromJson(const QString &key) {
    QList<QStringList> result;
    if (key.isEmpty()) {
        return result; // LCOV_EXCL_LINE
    }
    const auto raw =
        QSettings(getConfigFilePath(), QSettings::IniFormat).value(key).toString().trimmed();
    if (raw.isEmpty()) {
        return result;
    }
    const auto doc = QJsonDocument::fromJson(raw.toUtf8());
    if (!doc.isArray()) {
        return result;
    }
    const auto root = doc.array();
    for (const auto &val : root) {
        if (!val.isArray()) {
            continue;
        }
        QStringList argv;
        for (const auto &argVal : val.toArray()) {
            if (argVal.isString()) {
                argv.append(argVal.toString());
            }
        }
        if (!argv.isEmpty()) {
            result.append(argv);
        }
    }
    return result;
}

static QString launchCommandsKey(const QString &desktopPath, const QString &profileName) {
    if (profileName.isEmpty()) {
        return desktopPath;
    }
    static const auto kFmt = QStringLiteral("%1|%2");
    return kFmt.arg(desktopPath, profileName);
}

QList<QStringList> getPreLaunchCommands(const QString &desktopPath, const QString &profileName) {
    if (desktopPath.isEmpty()) {
        return {};
    }
    static const auto kFmt = QStringLiteral("PreLaunchCommands/%1");
    auto commands = readCommandListFromJson(kFmt.arg(launchCommandsKey(desktopPath, profileName)));
    if (commands.isEmpty() && !profileName.isEmpty()) {
        commands = readCommandListFromJson(kFmt.arg(desktopPath));
    }
    return commands;
}

QList<QStringList> getPostLaunchCommands(const QString &desktopPath, const QString &profileName) {
    if (desktopPath.isEmpty()) {
        return {};
    }
    static const auto kFmt = QStringLiteral("PostLaunchCommands/%1");
    auto commands = readCommandListFromJson(kFmt.arg(launchCommandsKey(desktopPath, profileName)));
    if (commands.isEmpty() && !profileName.isEmpty()) {
        commands = readCommandListFromJson(kFmt.arg(desktopPath));
    }
    return commands;
}

// LCOV_EXCL_START
void runLaunchHookCommand(const QStringList &argv, bool wait) {
    if (argv.isEmpty() || argv.first().isEmpty()) {
        return;
    }
    const auto program = argv.first();
    const auto args = argv.mid(1);
    if (wait) {
        QProcess proc;
        proc.setProgram(program);
        proc.setArguments(args);
        proc.start(QProcess::ReadOnly);
        proc.waitForFinished(30000);
    } else {
        QProcess::startDetached(program, args);
    }
}

void runPreLaunchCommands(const QString &desktopPath, const QString &profileName) {
    for (const auto &argv : getPreLaunchCommands(desktopPath, profileName)) {
        runLaunchHookCommand(argv, true);
    }
}

void runPostLaunchCommands(const QString &desktopPath, const QString &profileName) {
    for (const auto &argv : getPostLaunchCommands(desktopPath, profileName)) {
        runLaunchHookCommand(argv, false);
    }
}
// LCOV_EXCL_STOP

QString getChromeProfileDisplayName(const QString &desktopPath, const QString &profileId) {
    if (profileId == kGuest) {
        return QCoreApplication::translate("BrowserChooser", "Guest");
    }
    const auto userDataDir = getChromeUserDataDir(desktopPath);
    if (userDataDir.isEmpty() || !QFile::exists(kFmtLocalState.arg(userDataDir))) {
        return {};
    }
    return getChromeProfileDisplayNameFromUserDataDir(userDataDir, profileId);
}

QString getChromeProfilePicturePath(const QString &desktopPath, const QString &profileId) {
    if (profileId == kGuest) {
        return {};
    }
    const auto userDataDir = getChromeUserDataDir(desktopPath);
    if (userDataDir.isEmpty() || !QFile::exists(kFmtLocalState.arg(userDataDir))) {
        return {};
    }
    return getChromeProfilePicturePathFromUserDataDir(userDataDir, profileId);
}
