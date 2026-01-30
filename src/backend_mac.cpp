#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "backend.h"
#include "browseroption.h"
#include "desktopentry.h"
#include "firefox_profile.h"

namespace {

bool appHandlesHttpHttps(const QString &bundlePath) {
    const auto plistPath = bundlePath + QStringLiteral("/Contents/Info.plist");
    if (!QFile::exists(plistPath)) {
        return false;
    }
    QProcess proc;
    proc.setProgram(QStringLiteral("plutil"));
    proc.setArguments({QStringLiteral("-convert"),
                       QStringLiteral("json"),
                       QStringLiteral("-r"),
                       QStringLiteral("-o"),
                       QStringLiteral("-"),
                       plistPath});
    proc.start(QProcess::ReadOnly);
    if (!proc.waitForFinished(5000) || proc.exitStatus() != QProcess::NormalExit ||
        proc.exitCode() != 0) {
        return false;
    }
    const auto json = QJsonDocument::fromJson(proc.readAllStandardOutput());
    if (!json.isObject()) {
        return false;
    }
    const auto root = json.object();
    const auto urlTypes = root.value(QStringLiteral("CFBundleURLTypes")).toArray();
    for (const auto &typeVal : urlTypes) {
        const auto type = typeVal.toObject();
        const auto schemes = type.value(QStringLiteral("CFBundleURLSchemes")).toArray();
        for (const auto &schemeVal : schemes) {
            const auto scheme = schemeVal.toString().toLower();
            if (scheme == QStringLiteral("http") || scheme == QStringLiteral("https")) {
                return true;
            }
        }
    }
    return false;
}

QString getChromiumConfigDirForBundle(const QString &bundlePath) {
    const auto plistPath = bundlePath + QStringLiteral("/Contents/Info.plist");
    if (!QFile::exists(plistPath)) {
        return {};
    }
    QProcess proc;
    proc.setProgram(QStringLiteral("plutil"));
    proc.setArguments({QStringLiteral("-convert"),
                       QStringLiteral("json"),
                       QStringLiteral("-r"),
                       QStringLiteral("-o"),
                       QStringLiteral("-"),
                       plistPath});
    proc.start(QProcess::ReadOnly);
    if (!proc.waitForFinished(5000) || proc.exitStatus() != QProcess::NormalExit ||
        proc.exitCode() != 0) {
        return {};
    }
    const auto json = QJsonDocument::fromJson(proc.readAllStandardOutput());
    if (!json.isObject()) {
        return {};
    }
    const auto root = json.object();
    auto dirName = root.value(QStringLiteral("CrProductDirName")).toString();
    if (!dirName.isEmpty()) {
        return QDir::homePath() + QStringLiteral("/Library/Application Support/") + dirName;
    }
    auto appName = root.value(QStringLiteral("CFBundleName")).toString();
    if (appName.isEmpty()) {
        appName = root.value(QStringLiteral("CFBundleExecutable")).toString();
    }
    if (appName.isEmpty()) {
        return {};
    }
    if (appName == QStringLiteral("Google Chrome")) {
        dirName = QStringLiteral("Google/Chrome");
    } else if (appName == QStringLiteral("Chromium")) {
        dirName = QStringLiteral("Chromium");
    } else if (appName.contains(QStringLiteral("Chrome"))) {
        dirName = QStringLiteral("Google/") + appName;
    } else if (appName.contains(QStringLiteral("Brave"), Qt::CaseInsensitive)) {
        dirName = QStringLiteral("BraveSoftware/Brave-Browser");
    } else if (appName.contains(QStringLiteral("Edge"), Qt::CaseInsensitive)) {
        dirName = QStringLiteral("Microsoft Edge");
    } else {
        return {};
    }
    return QDir::homePath() + QStringLiteral("/Library/Application Support/") + dirName;
}

QString getFirefoxConfigDir() {
    return QDir::homePath() + QStringLiteral("/Library/Application Support/Firefox");
}

bool isFirefoxBundle(const QString &bundlePath) {
    const auto baseName = QFileInfo(bundlePath).completeBaseName();
    return baseName.contains(QStringLiteral("Firefox"), Qt::CaseInsensitive);
}

bool isSafariBundle(const QString &bundlePath) {
    const auto baseName = QFileInfo(bundlePath).completeBaseName();
    return baseName.compare(QStringLiteral("Safari"), Qt::CaseInsensitive) == 0;
}

bool isWebBrowser(const QString &bundlePath) {
    return isSafariBundle(bundlePath) || appHandlesHttpHttps(bundlePath);
}

// Reads a comma-separated list from the config file (used for Advanced/hideProfileBrowsers
// and Advanced/hideBrowsers). Identifiers can be bundle names (e.g. Firefox), bundle IDs
// (e.g. com.apple.Safari), or full canonical paths.
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

QString getCanonicalBrowserPath(const BrowserOption &option) {
    const auto bundlePath = option.desktopPath();
    if (bundlePath.isEmpty() || !bundlePath.endsWith(QStringLiteral(".app"))) {
        return {};
    }
    const auto execName = option.entry().exec();
    if (execName.isEmpty()) {
        return {};
    }
    const auto binaryPath = bundlePath + QStringLiteral("/Contents/MacOS/") + execName;
    if (!QFile::exists(binaryPath)) {
        return {};
    }
    const auto canonical = QFileInfo(binaryPath).canonicalFilePath();
    return canonical.isEmpty() ? binaryPath : canonical;
}

QString bundleName(const QString &bundlePath) {
    return QFileInfo(bundlePath).completeBaseName();
}

QString getBundleIdentifier(const QString &bundlePath) {
    const auto plistPath = bundlePath + QStringLiteral("/Contents/Info.plist");
    if (!QFile::exists(plistPath)) {
        return {};
    }
    QProcess proc;
    proc.setProgram(QStringLiteral("plutil"));
    proc.setArguments({QStringLiteral("-convert"),
                       QStringLiteral("json"),
                       QStringLiteral("-r"),
                       QStringLiteral("-o"),
                       QStringLiteral("-"),
                       plistPath});
    proc.start(QProcess::ReadOnly);
    if (!proc.waitForFinished(5000) || proc.exitStatus() != QProcess::NormalExit ||
        proc.exitCode() != 0) {
        return {};
    }
    const auto json = QJsonDocument::fromJson(proc.readAllStandardOutput());
    if (!json.isObject()) {
        return {};
    }
    return json.object().value(QStringLiteral("CFBundleIdentifier")).toString();
}

} // anonymous namespace

QList<BrowserOption> getBrowsers(IncludeNoDisplay) {
    QList<BrowserOption> options;
    const QStringList appDirs = {
        QStringLiteral("/Applications"),
        QDir::homePath() + QStringLiteral("/Applications"),
    };
    const auto hideProfileBrowsers =
        readCommaSeparatedList(QStringLiteral("Advanced/hideProfileBrowsers"));
    const auto firefoxConfigDir = getFirefoxConfigDir();
    const auto firefoxProfiles = QDir(firefoxConfigDir).exists() ?
                                     getFirefoxProfiles(firefoxConfigDir) :
                                     QList<FirefoxProfilePair>();
    for (const auto &appDir : appDirs) {
        QDir dir(appDir);
        if (!dir.exists()) {
            continue;
        }
        const auto entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &name : entries) {
            if (!name.endsWith(QStringLiteral(".app"))) {
                continue;
            }
            const auto bundlePath = dir.absoluteFilePath(name);
            if (!isWebBrowser(bundlePath)) {
                continue;
            }
            DesktopEntry entry;
            if (!entry.parseAppBundle(bundlePath)) {
                continue;
            }
            const auto nameForBundle = bundleName(bundlePath);
            const auto binaryPath =
                entry.exec().isEmpty() ?
                    QString() :
                    bundlePath + QStringLiteral("/Contents/MacOS/") + entry.exec();
            const auto canonicalBinaryPath =
                binaryPath.isEmpty() || !QFile::exists(binaryPath) ?
                    QString() :
                    (QFileInfo(binaryPath).canonicalFilePath().isEmpty() ?
                         binaryPath :
                         QFileInfo(binaryPath).canonicalFilePath());
            const auto bundleId = getBundleIdentifier(bundlePath);
            const bool skipFirefoxProfiles =
                isFirefoxBundle(bundlePath) &&
                (listContainsIdentifier(hideProfileBrowsers, nameForBundle) ||
                 listContainsIdentifier(hideProfileBrowsers, canonicalBinaryPath) ||
                 listContainsIdentifier(hideProfileBrowsers, bundleId));
            const bool useFirefoxProfiles =
                isFirefoxBundle(bundlePath) && !skipFirefoxProfiles && !firefoxProfiles.isEmpty();
            if (useFirefoxProfiles) {
                const bool singleProfile = firefoxProfiles.size() == 1;
                for (const auto &pair : firefoxProfiles) {
                    options.append(
                        BrowserOption(entry, pair.first, pair.second, singleProfile, true));
                }
            } else {
                options.append(BrowserOption(entry, QString(), QString(), true, false));
            }
        }
    }
    std::ranges::sort(options, [](const BrowserOption &a, const BrowserOption &b) {
        return a.displayName().compare(b.displayName(), Qt::CaseInsensitive) < 0;
    });
    const auto hideBrowsers = readCommaSeparatedList(QStringLiteral("Advanced/hideBrowsers"));
    if (!hideBrowsers.isEmpty()) {
        options.removeIf([&hideBrowsers](const BrowserOption &opt) {
            const auto name = bundleName(opt.desktopPath());
            const auto canonicalPath = getCanonicalBrowserPath(opt);
            const auto bundleId = getBundleIdentifier(opt.desktopPath());
            return listContainsIdentifier(hideBrowsers, name) ||
                   listContainsIdentifier(hideBrowsers, canonicalPath) ||
                   listContainsIdentifier(hideBrowsers, bundleId);
        });
    }
    return options;
}

void launchBrowser(const BrowserOption &option, const QStringList &urls) {
    const auto bundlePath = option.desktopPath();
    if (!bundlePath.endsWith(QStringLiteral(".app")) || !QDir(bundlePath).exists()) {
        return;
    }
    QStringList args{QStringLiteral("-a"), bundlePath};
    QStringList appArgs;
    if (!option.profileName().isEmpty()) {
        if (isFirefoxBundle(bundlePath)) {
            appArgs << QStringLiteral("-P") << option.profileName();
        } else {
            appArgs << QStringLiteral("--profile-directory=") + option.profileName();
        }
    }
    appArgs << urls;
    if (!appArgs.isEmpty()) {
        args << QStringLiteral("--args") << appArgs;
    }
    QProcess::startDetached(QStringLiteral("open"), args);
}

QString getCommandLineForDisplay(const BrowserOption &option, const QString &url) {
    const auto bundlePath = option.desktopPath();
    QString cmd = QStringLiteral("open -a \"%1\"").arg(bundlePath);
    QStringList appArgs;
    if (!option.profileName().isEmpty()) {
        if (isFirefoxBundle(bundlePath)) {
            appArgs << QStringLiteral("-P") << option.profileName();
        } else {
            appArgs << QStringLiteral("--profile-directory=") + option.profileName();
        }
    }
    if (!url.isEmpty()) {
        appArgs << url;
    }
    if (!appArgs.isEmpty()) {
        cmd += QStringLiteral(" --args");
        for (const auto &arg : appArgs) {
            cmd += QLatin1Char(' ') + QStringLiteral("\"%1\"").arg(arg);
        }
    }
    return cmd;
}

QString getExecutablePath(const BrowserOption &option) {
    const auto e = option.entry();
    if (!e.isValid()) {
        return {};
    }
    return e.exec();
}

QString getConfigFilePath() {
    const auto configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + QStringLiteral("/browserchooserrc");
}

QString getChromeUserDataDir(const QString &desktopPath) {
    if (!desktopPath.endsWith(QStringLiteral(".app")) || !QDir(desktopPath).exists()) {
        return {};
    }
    return getChromiumConfigDirForBundle(desktopPath);
}
