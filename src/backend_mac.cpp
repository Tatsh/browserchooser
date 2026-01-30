#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
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

} // anonymous namespace

QList<BrowserOption> getBrowsers(IncludeNoDisplay) {
    QList<BrowserOption> options;
    const QStringList appDirs = {
        QStringLiteral("/Applications"),
        QDir::homePath() + QStringLiteral("/Applications"),
    };
    const auto firefoxConfigDir = getFirefoxConfigDir();
    const auto firefoxProfiles = QDir(firefoxConfigDir).exists()
                                    ? getFirefoxProfiles(firefoxConfigDir)
                                    : QList<FirefoxProfilePair>();
    const bool hasFirefoxProfiles = !firefoxProfiles.isEmpty();
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
            if (isFirefoxBundle(bundlePath) && hasFirefoxProfiles) {
                const bool singleProfile = firefoxProfiles.size() == 1;
                for (const auto &pair : firefoxProfiles) {
                    options.append(BrowserOption(
                        entry, pair.first, pair.second, singleProfile, true));
                }
            } else {
                options.append(BrowserOption(entry, QString(), QString(), true, false));
            }
        }
    }
    std::ranges::sort(options, [](const BrowserOption &a, const BrowserOption &b) {
        return a.displayName().compare(b.displayName(), Qt::CaseInsensitive) < 0;
    });
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
        appArgs << QStringLiteral("-P") << option.profileName();
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
        appArgs << QStringLiteral("-P") << option.profileName();
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
