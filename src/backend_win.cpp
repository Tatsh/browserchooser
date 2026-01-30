#include <ranges>

#ifdef Q_OS_WIN
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QSet>

#include "backend.h"
#include "browseroption.h"
#include "desktopentry.h"
#include "firefox_profile.h"

#include <windows.h>

namespace {

constexpr const wchar_t *kAppPathsKey =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths";

const QStringList kBrowserExeNames = {
    QStringLiteral("chrome.exe"),
    QStringLiteral("firefox.exe"),
    QStringLiteral("msedge.exe"),
    QStringLiteral("brave.exe"),
    QStringLiteral("opera.exe"),
    QStringLiteral("iexplore.exe"),
    QStringLiteral("chromium.exe"),
};

QString getAppPathFromRegistry(HKEY hive, const QString &exeName) {
    const auto keyPath = QString::fromWCharArray(kAppPathsKey) + QLatin1Char('\\') + exeName;
    const auto keyPathW = keyPath.toStdWString();
    HKEY key = nullptr;
    if (RegOpenKeyExW(hive, keyPathW.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return {};
    }
    wchar_t pathBuf[MAX_PATH] = {};
    DWORD pathLen = sizeof(pathBuf);
    const auto err = RegQueryValueExW(key, nullptr, nullptr, nullptr,
                                      reinterpret_cast<LPBYTE>(pathBuf), &pathLen);
    RegCloseKey(key);
    if (err != ERROR_SUCCESS) {
        return {};
    }
    return QString::fromWCharArray(pathBuf);
}

QStringList discoverBrowserPaths() {
    QSet<QString> seen;
    QStringList paths;
    for (const auto &exeName : kBrowserExeNames) {
        for (auto hive : {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE}) {
            auto path = getAppPathFromRegistry(hive, exeName);
            if (path.isEmpty()) {
                continue;
            }
            path = QDir::fromNativeSeparators(path);
            if (!QFile::exists(path) || seen.contains(path)) {
                continue;
            }
            seen.insert(path);
            paths.append(path);
        }
    }
    return paths;
}

QString getChromeUserDataDirForExe(const QString &exePath) {
    const auto exeDir = QFileInfo(exePath).absolutePath();
    const auto dirName = QFileInfo(exeDir).fileName();
    if (dirName.compare(QStringLiteral("Application"), Qt::CaseInsensitive) == 0) {
        const auto userDataDir = QDir::cleanPath(QDir(exeDir).filePath(QStringLiteral("../User Data")));
        if (QFile::exists(userDataDir + QStringLiteral("/Local State"))) {
            return userDataDir;
        }
    }
    QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
    if (localAppData.isEmpty()) {
        localAppData = QDir::homePath() + QStringLiteral("/AppData/Local");
    }
    localAppData = QDir::fromNativeSeparators(localAppData);
    const auto baseName = QFileInfo(exePath).completeBaseName().toLower();
    QString subPath;
    if (baseName == QStringLiteral("chrome")) {
        subPath = QStringLiteral("Google/Chrome/User Data");
    } else if (baseName == QStringLiteral("msedge")) {
        subPath = QStringLiteral("Microsoft/Edge/User Data");
    } else if (baseName == QStringLiteral("brave")) {
        subPath = QStringLiteral("BraveSoftware/Brave-Browser/User Data");
    } else if (baseName == QStringLiteral("chromium")) {
        subPath = QStringLiteral("Chromium/User Data");
    } else {
        return {};
    }
    return localAppData + QLatin1Char('/') + subPath;
}

QString getFirefoxConfigDir() {
    const auto appData = qEnvironmentVariable("APPDATA");
    if (appData.isEmpty()) {
        return QDir::homePath() + QStringLiteral("/AppData/Roaming/Mozilla/Firefox");
    }
    return QDir::fromNativeSeparators(appData) + QStringLiteral("/Mozilla/Firefox");
}

bool isFirefoxExe(const QString &exePath) {
    return QFileInfo(exePath).fileName().compare(QStringLiteral("firefox.exe"),
                                                  Qt::CaseInsensitive)
           == 0;
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

} // anonymous namespace

QList<BrowserOption> getBrowsers(IncludeNoDisplay) {
    QList<BrowserOption> options;
    const auto paths = discoverBrowserPaths();
    const auto firefoxConfigDir = getFirefoxConfigDir();
    const auto firefoxProfiles = QDir(firefoxConfigDir).exists()
                                    ? getFirefoxProfiles(firefoxConfigDir)
                                    : QList<FirefoxProfilePair>();
    const bool hasFirefoxProfiles = !firefoxProfiles.isEmpty();
    for (const auto &exePath : paths) {
        DesktopEntry entry;
        if (!entry.parseFromExecutable(exePath)) {
            continue;
        }
        if (isFirefoxExe(exePath) && hasFirefoxProfiles) {
            const bool singleProfile = firefoxProfiles.size() == 1;
            for (const auto &pair : firefoxProfiles) {
                options.append(
                    BrowserOption(entry, pair.first, pair.second, singleProfile, true));
            }
        } else {
            options.append(BrowserOption(entry, QString(), QString(), true, false));
        }
    }
    std::ranges::sort(options, [](const BrowserOption &a, const BrowserOption &b) {
        return a.displayName().compare(b.displayName(), Qt::CaseInsensitive) < 0;
    });
    // Secret option: set [Advanced] ShowIExplorer=true in the config file to show IE.
    const bool showIExplorer =
        QSettings(getConfigFilePath(), QSettings::IniFormat)
            .value(QStringLiteral("Advanced/ShowIExplorer"), false)
            .toBool();
    if (!showIExplorer) {
        options.removeIf([](const BrowserOption &opt) {
            return opt.desktopPath().endsWith(QStringLiteral("iexplore.exe"),
                                              Qt::CaseInsensitive);
        });
    }
    return options;
}

void launchBrowser(const BrowserOption &option, const QStringList &urls) {
    const auto exePath = option.desktopPath();
    if (!exePath.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive) ||
        !QFile::exists(exePath)) {
        return;
    }
    QStringList args;
    if (!option.profileName().isEmpty()) {
        args << QStringLiteral("-P") << option.profileName();
    }
    args << urls;
    QProcess::startDetached(exePath, args);
}

QString getCommandLineForDisplay(const BrowserOption &option, const QString &url) {
    const auto exePath = option.desktopPath();
    QString cmd = quoteArg(exePath);
    if (!option.profileName().isEmpty()) {
        cmd += QLatin1Char(' ') + quoteArg(QStringLiteral("-P"))
             + QLatin1Char(' ') + quoteArg(option.profileName());
    }
    if (!url.isEmpty()) {
        cmd += QLatin1Char(' ') + quoteArg(url);
    }
    return cmd;
}

QString getExecutablePath(const BrowserOption &option) {
    return option.desktopPath();
}

QString getConfigFilePath() {
    const auto configDir =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + QStringLiteral("/browserchooserrc");
}

QString getChromeUserDataDir(const QString &desktopPath) {
    if (!desktopPath.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive)) {
        return {};
    }
    return getChromeUserDataDirForExe(desktopPath);
}

#endif // Q_OS_WIN
