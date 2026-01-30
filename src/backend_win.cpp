#include <ranges>

#ifdef Q_OS_WIN
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QSet>

#include "backend.h"
#include "browseroption.h"
#include "chrome_profile.h"
#include "desktopentry.h"

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
    for (const auto &exePath : paths) {
        DesktopEntry entry;
        if (!entry.parseFromExecutable(exePath)) {
            continue;
        }
        options.append(BrowserOption(entry, QString(), QString(), true, false));
    }
    std::ranges::sort(options, [](const BrowserOption &a, const BrowserOption &b) {
        return a.displayName().compare(b.displayName(), Qt::CaseInsensitive) < 0;
    });
    return options;
}

void launchBrowser(const BrowserOption &option, const QStringList &urls) {
    const auto exePath = option.desktopPath();
    if (!exePath.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive) ||
        !QFile::exists(exePath)) {
        return;
    }
    QStringList args;
    if (!urls.isEmpty()) {
        args << urls;
    }
    QProcess::startDetached(exePath, args);
}

QString getCommandLineForDisplay(const BrowserOption &option, const QString &url) {
    const auto exePath = option.desktopPath();
    if (url.isEmpty()) {
        return quoteArg(exePath);
    }
    return quoteArg(exePath) + QLatin1Char(' ') + quoteArg(url);
}

QString getExecutablePath(const BrowserOption &option) {
    return option.desktopPath();
}

QString getConfigFilePath() {
    const auto configDir =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + QStringLiteral("/browserchooserrc");
}

QString getChromeProfileDisplayName(const QString &desktopPath, const QString &profileId) {
    if (profileId == QStringLiteral("Guest")) {
        return QStringLiteral("Guest");
    }
    if (!desktopPath.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive)) {
        return {};
    }
    const auto userDataDir = getChromeUserDataDirForExe(desktopPath);
    if (userDataDir.isEmpty() || !QFile::exists(userDataDir + QStringLiteral("/Local State"))) {
        return {};
    }
    return getChromeProfileDisplayNameFromUserDataDir(userDataDir, profileId);
}

#endif // Q_OS_WIN
