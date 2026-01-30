#include <ranges>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonValue>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "backend.h"
#include "chrome_profile.h"

QString getConfigFilePath() {
    const auto configDir =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + QStringLiteral("/browserchooserrc");
}

QStringList readCommaSeparatedList(const QString &key) {
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

QString quoteArg(const QString &arg) {
    if (arg.isEmpty()) {
        return QStringLiteral("\"\"");
    }
    auto needQuote = false;
    for (const auto c : arg) {
        if (c == QLatin1Char(' ') || c == QLatin1Char('"') || c == QLatin1Char('\\')) {
            needQuote = true;
            break;
        }
    }
    if (!needQuote) {
        return arg;
    }
    QString result;
    result.reserve(arg.size() + 2);
    result += QLatin1Char('"');
    for (const auto c : arg) {
        if (c == QLatin1Char('"')) {
            result += QStringLiteral("\\\"");
        } else if (c == QLatin1Char('\\')) {
            result += QStringLiteral("\\\\");
        } else {
            result += c;
        }
    }
    result += QLatin1Char('"');
    return result;
}

void sortBrowserOptionsByDisplayName(QList<BrowserOption> &options) {
    std::ranges::sort(options, [](const BrowserOption &a, const BrowserOption &b) {
        return a.displayName().compare(b.displayName(), Qt::CaseInsensitive) < 0;
    });
}

static QList<QStringList> readCommandListFromJson(const QString &key) {
    QList<QStringList> result;
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
    return desktopPath + QLatin1Char('|') + profileName;
}

QList<QStringList> getPreLaunchCommands(const QString &desktopPath,
                                         const QString &profileName) {
    const auto group = QStringLiteral("PreLaunchCommands/");
    auto commands = readCommandListFromJson(group + launchCommandsKey(desktopPath, profileName));
    if (commands.isEmpty() && !profileName.isEmpty()) {
        commands = readCommandListFromJson(group + desktopPath);
    }
    return commands;
}

QList<QStringList> getPostLaunchCommands(const QString &desktopPath,
                                          const QString &profileName) {
    const auto group = QStringLiteral("PostLaunchCommands/");
    auto commands = readCommandListFromJson(group + launchCommandsKey(desktopPath, profileName));
    if (commands.isEmpty() && !profileName.isEmpty()) {
        commands = readCommandListFromJson(group + desktopPath);
    }
    return commands;
}

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

QString getChromeProfileDisplayName(const QString &desktopPath, const QString &profileId) {
    if (profileId == QStringLiteral("Guest")) {
        return QCoreApplication::translate("BrowserChooser", "Guest");
    }
    const auto userDataDir = getChromeUserDataDir(desktopPath);
    if (userDataDir.isEmpty() || !QFile::exists(userDataDir + QStringLiteral("/Local State"))) {
        return {};
    }
    return getChromeProfileDisplayNameFromUserDataDir(userDataDir, profileId);
}

QString getChromeProfilePicturePath(const QString &desktopPath, const QString &profileId) {
    if (profileId == QStringLiteral("Guest")) {
        return {};
    }
    const auto userDataDir = getChromeUserDataDir(desktopPath);
    if (userDataDir.isEmpty() || !QFile::exists(userDataDir + QStringLiteral("/Local State"))) {
        return {};
    }
    return getChromeProfilePicturePathFromUserDataDir(userDataDir, profileId);
}
