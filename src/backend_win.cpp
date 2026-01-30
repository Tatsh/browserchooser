#include <ranges>

#ifdef Q_OS_WIN
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "backend.h"
#include "browseroption.h"
#include "desktopentry.h"
#include "firefox_profile.h"

#include <windows.h>

namespace {

constexpr const wchar_t *kAppPathsKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths";

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
    const auto err = RegQueryValueExW(
        key, nullptr, nullptr, nullptr, reinterpret_cast<LPBYTE>(pathBuf), &pathLen);
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
        const auto userDataDir =
            QDir::cleanPath(QDir(exeDir).filePath(QStringLiteral("../User Data")));
        if (QFile::exists(userDataDir + QStringLiteral("/Local State"))) {
            return userDataDir;
        }
    }
    const QString localAppData =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (localAppData.isEmpty()) {
        return {};
    }
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
    const auto configRoot =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configRoot.isEmpty()) {
        return {};
    }
    return configRoot + QStringLiteral("/Mozilla/Firefox");
}

bool isFirefoxExe(const QString &exePath) {
    return QFileInfo(exePath).fileName().compare(QStringLiteral("firefox.exe"),
                                                 Qt::CaseInsensitive) == 0;
}

QString getCanonicalBrowserPath(const BrowserOption &option) {
    const auto path = option.desktopPath();
    if (path.isEmpty()) {
        return {};
    }
    const auto canonical = QFileInfo(path).canonicalFilePath();
    return canonical.isEmpty() ? path : canonical;
}

QString exeBaseName(const QString &exePath) {
    return QFileInfo(exePath).completeBaseName();
}

} // anonymous namespace

QList<BrowserOption> getBrowsers(IncludeNoDisplay) {
    QList<BrowserOption> options;
    const auto paths = discoverBrowserPaths();
    const auto hideProfileBrowsers =
        readCommaSeparatedList(QStringLiteral("Advanced/hideProfileBrowsers"));
    const auto firefoxConfigDir = getFirefoxConfigDir();
    const auto firefoxProfiles = QDir(firefoxConfigDir).exists() ?
                                     getFirefoxProfiles(firefoxConfigDir) :
                                     QList<FirefoxProfilePair>();
    for (const auto &exePath : paths) {
        DesktopEntry entry;
        if (!entry.parseFromExecutable(exePath)) {
            continue;
        }
        const auto baseName = exeBaseName(exePath);
        const auto canonicalExePath = QFileInfo(exePath).canonicalFilePath();
        const auto resolvedExePath = canonicalExePath.isEmpty() ? exePath : canonicalExePath;
        const bool skipFirefoxProfiles =
            isFirefoxExe(exePath) &&
            (listContainsIdentifier(hideProfileBrowsers, QStringLiteral("firefox")) ||
             listContainsIdentifier(hideProfileBrowsers, resolvedExePath));
        const bool useFirefoxProfiles =
            isFirefoxExe(exePath) && !skipFirefoxProfiles && !firefoxProfiles.isEmpty();
        if (useFirefoxProfiles) {
            const bool singleProfile = firefoxProfiles.size() == 1;
            for (const auto &pair : firefoxProfiles) {
                options.append(BrowserOption(entry, pair.first, pair.second, singleProfile, true));
            }
        } else {
            options.append(BrowserOption(entry, QString(), QString(), true, false));
        }
    }
    sortBrowserOptionsByDisplayName(options);
    const QVariant hideBrowsersVar = QSettings(getConfigFilePath(), QSettings::IniFormat)
                                         .value(QStringLiteral("Advanced/hideBrowsers"));
    QStringList hideBrowsers = hideBrowsersVar.isNull() || !hideBrowsersVar.isValid() ?
                                   QStringList{QStringLiteral("iexplore")} :
                                   readCommaSeparatedList(QStringLiteral("Advanced/hideBrowsers"));
    options.removeIf([&hideBrowsers](const BrowserOption &opt) {
        const auto baseName = exeBaseName(opt.desktopPath());
        const auto canonicalPath = getCanonicalBrowserPath(opt);
        return listContainsIdentifier(hideBrowsers, baseName) ||
               listContainsIdentifier(hideBrowsers, canonicalPath);
    });
    return options;
}

void launchBrowser(const BrowserOption &option, const QStringList &urls) {
    runPreLaunchCommands(option.desktopPath(), option.profileName());
    const auto exePath = option.desktopPath();
    if (!exePath.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive) || !QFile::exists(exePath)) {
        return;
    }
    QStringList args;
    if (!option.profileName().isEmpty()) {
        if (isFirefoxExe(exePath)) {
            args << QStringLiteral("-P") << option.profileName();
        } else {
            args << QStringLiteral("--profile-directory=") + option.profileName();
        }
    }
    args << urls;
    QProcess::startDetached(exePath, args);
    runPostLaunchCommands(option.desktopPath(), option.profileName());
}

QString getCommandLineForDisplay(const BrowserOption &option, const QString &url) {
    const auto exePath = option.desktopPath();
    QString cmd = quoteArg(exePath);
    if (!option.profileName().isEmpty()) {
        if (isFirefoxExe(exePath)) {
            cmd += QLatin1Char(' ') + quoteArg(QStringLiteral("-P")) + QLatin1Char(' ') +
                   quoteArg(option.profileName());
        } else {
            cmd += QLatin1Char(' ') +
                   quoteArg(QStringLiteral("--profile-directory=") + option.profileName());
        }
    }
    if (!url.isEmpty()) {
        cmd += QLatin1Char(' ') + quoteArg(url);
    }
    return cmd;
}

QString getExecutablePath(const BrowserOption &option) {
    return option.desktopPath();
}

QString getChromeUserDataDir(const QString &desktopPath) {
    if (!desktopPath.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive)) {
        return {};
    }
    return getChromeUserDataDirForExe(desktopPath);
}

#endif // Q_OS_WIN
