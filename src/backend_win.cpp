#include <ranges>

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
#include "string_constants.h"

#include <windows.h>

namespace {

constexpr const wchar_t *kAppPathsKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths";
static const auto kFmtRegKeyPath = QStringLiteral("%1\\%2");
static const auto kFmtMozillaFirefox = QStringLiteral("%1/Mozilla/Firefox");
static const auto kFmtTwoArgs = QStringLiteral(" %1 %2");
static const auto kFmtOneArg = QStringLiteral(" %1");
static const auto kSuffixExe = QStringLiteral(".exe");
static const auto kFirefoxExe = QStringLiteral("firefox.exe");
static const auto kExeChrome = QStringLiteral("chrome");
static const auto kExeMsedge = QStringLiteral("msedge");
static const auto kExeBrave = QStringLiteral("brave");
static const auto kExeChromium = QStringLiteral("chromium");
static const auto kSubPathGoogleChrome = QStringLiteral("Google/Chrome/User Data");
static const auto kSubPathMicrosoftEdge = QStringLiteral("Microsoft/Edge/User Data");
static const auto kSubPathBraveBrowser = QStringLiteral("BraveSoftware/Brave-Browser/User Data");
static const auto kSubPathChromium = QStringLiteral("Chromium/User Data");

const QStringList kBrowserExeNames = {
    QStringLiteral("chrome.exe"),
    kFirefoxExe,
    QStringLiteral("msedge.exe"),
    QStringLiteral("brave.exe"),
    QStringLiteral("opera.exe"),
    QStringLiteral("iexplore.exe"),
    QStringLiteral("chromium.exe"),
};

QString getAppPathFromRegistry(HKEY hive, const QString &exeName) {
    const auto keyPath = kFmtRegKeyPath.arg(QString::fromWCharArray(kAppPathsKey), exeName);
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
    static const auto kApplication = QStringLiteral("Application");
    static const auto kUserDataRelative = QStringLiteral("../User Data");
    if (dirName.compare(kApplication, Qt::CaseInsensitive) == 0) {
        const auto userDataDir = QDir::cleanPath(QDir(exeDir).filePath(kUserDataRelative));
        if (QFile::exists(kFmtLocalState.arg(userDataDir))) {
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
    if (baseName == kExeChrome) {
        subPath = kSubPathGoogleChrome;
    } else if (baseName == kExeMsedge) {
        subPath = kSubPathMicrosoftEdge;
    } else if (baseName == kExeBrave) {
        subPath = kSubPathBraveBrowser;
    } else if (baseName == kExeChromium) {
        subPath = kSubPathChromium;
    } else {
        return {};
    }
    return kFmtPath.arg(localAppData, subPath);
}

QString getFirefoxConfigDir() {
    const auto configRoot = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configRoot.isEmpty()) {
        return {};
    }
    return kFmtMozillaFirefox.arg(configRoot);
}

bool isFirefoxExe(const QString &exePath) {
    return QFileInfo(exePath).fileName().compare(kFirefoxExe, Qt::CaseInsensitive) == 0;
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

// Windows: double-quote style; " and \ must be escaped (CreateProcess rules).
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

QList<BrowserOption> getBrowsers(IncludeNoDisplay) {
    QList<BrowserOption> options;
    const auto paths = discoverBrowserPaths();
    const auto hideProfileBrowsers = readCommaSeparatedList(kKeyHideProfileBrowsers);
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
            isFirefoxExe(exePath) && (listContainsIdentifier(hideProfileBrowsers, kFirefox) ||
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
    static const auto kIexplore = QStringLiteral("iexplore");
    const QVariant hideBrowsersVar =
        QSettings(getConfigFilePath(), QSettings::IniFormat).value(kKeyHideBrowsers);
    QStringList hideBrowsers = hideBrowsersVar.isNull() || !hideBrowsersVar.isValid() ?
                                   QStringList{kIexplore} :
                                   readCommaSeparatedList(kKeyHideBrowsers);
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
    if (!exePath.endsWith(kSuffixExe, Qt::CaseInsensitive) || !QFile::exists(exePath)) {
        return;
    }
    QStringList args;
    if (!option.profileName().isEmpty()) {
        if (isFirefoxExe(exePath)) {
            args << kArgP << option.profileName();
        } else {
            args << kFmtProfileDirectory.arg(option.profileName());
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
            cmd += kFmtTwoArgs.arg(quoteArg(kArgP), quoteArg(option.profileName()));
        } else {
            cmd += kFmtOneArg.arg(quoteArg(kFmtProfileDirectory.arg(option.profileName())));
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
    if (!desktopPath.endsWith(kSuffixExe, Qt::CaseInsensitive)) {
        return {};
    }
    return getChromeUserDataDirForExe(desktopPath);
}
