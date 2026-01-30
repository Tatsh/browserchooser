#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>

#import <Foundation/Foundation.h>

#include "backend.h"
#include "browseroption.h"
#include "desktopentry.h"
#include "firefox_profile.h"

namespace {

static QString stringFromNSString(NSString *ns) {
    if (!ns || ![ns isKindOfClass:[NSString class]]) {
        return {};
    }
    return QString::fromUtf8([ns UTF8String]);
}

static NSDictionary *loadPlist(const QString &plistPath) {
    const QByteArray pathUtf8 = plistPath.toUtf8();
    NSString *path = [NSString stringWithUTF8String:pathUtf8.constData()];
    return [NSDictionary dictionaryWithContentsOfFile:path];
}

bool appHandlesHttpHttps(const QString &bundlePath) {
    const auto plistPath = bundlePath + QStringLiteral("/Contents/Info.plist");
    if (!QFile::exists(plistPath)) {
        return false;
    }
    @autoreleasepool {
        NSDictionary *root = loadPlist(plistPath);
        if (!root) {
            return false;
        }
        NSArray *urlTypes = root[@"CFBundleURLTypes"];
        if (!urlTypes || ![urlTypes isKindOfClass:[NSArray class]]) {
            return false;
        }
        for (id typeVal in urlTypes) {
            if (![typeVal isKindOfClass:[NSDictionary class]]) {
                continue;
            }
            NSDictionary *type = (NSDictionary *)typeVal;
            NSArray *schemes = type[@"CFBundleURLSchemes"];
            if (!schemes || ![schemes isKindOfClass:[NSArray class]]) {
                continue;
            }
            for (id schemeVal in schemes) {
                if (![schemeVal isKindOfClass:[NSString class]]) {
                    continue;
                }
                NSString *scheme = [(NSString *)schemeVal lowercaseString];
                if ([scheme isEqualToString:@"http"] || [scheme isEqualToString:@"https"]) {
                    return true;
                }
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
    const QString supportBase =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (supportBase.isEmpty()) {
        return {};
    }
    @autoreleasepool {
        NSDictionary *root = loadPlist(plistPath);
        if (!root) {
            return {};
        }
        NSString *crProductDir = root[@"CrProductDirName"];
        if (crProductDir && [crProductDir isKindOfClass:[NSString class]] &&
            [crProductDir length] > 0) {
            return supportBase + QLatin1Char('/') + stringFromNSString(crProductDir);
        }
        NSString *appName = root[@"CFBundleName"];
        if (!appName || ![appName isKindOfClass:[NSString class]] || [appName length] == 0) {
            appName = root[@"CFBundleExecutable"];
        }
        if (!appName || ![appName isKindOfClass:[NSString class]] || [appName length] == 0) {
            return {};
        }
        QString appNameQ = stringFromNSString(appName);
        QString dirName;
        if (appNameQ == QStringLiteral("Google Chrome")) {
            dirName = QStringLiteral("Google/Chrome");
        } else if (appNameQ == QStringLiteral("Chromium")) {
            dirName = QStringLiteral("Chromium");
        } else if (appNameQ.contains(QStringLiteral("Chrome"))) {
            dirName = QStringLiteral("Google/") + appNameQ;
        } else if (appNameQ.contains(QStringLiteral("Brave"), Qt::CaseInsensitive)) {
            dirName = QStringLiteral("BraveSoftware/Brave-Browser");
        } else if (appNameQ.contains(QStringLiteral("Edge"), Qt::CaseInsensitive)) {
            dirName = QStringLiteral("Microsoft Edge");
        } else {
            return {};
        }
        return supportBase + QLatin1Char('/') + dirName;
    }
}

QString getFirefoxConfigDir() {
    const auto supportBase =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (supportBase.isEmpty()) {
        return {};
    }
    return supportBase + QStringLiteral("/Firefox");
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
    @autoreleasepool {
        NSDictionary *root = loadPlist(plistPath);
        if (!root) {
            return {};
        }
        id value = root[@"CFBundleIdentifier"];
        if (![value isKindOfClass:[NSString class]]) {
            return {};
        }
        return stringFromNSString((NSString *)value);
    }
}

} // anonymous namespace

QList<BrowserOption> getBrowsers(IncludeNoDisplay) {
    QList<BrowserOption> options;
    // Use ApplicationsLocation for /Applications and ~/Applications on macOS.
    auto userAppDir =
        QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    if (userAppDir.isEmpty()) {
        const auto fallback = QDir::homePath() + QStringLiteral("/Applications");
        if (QDir(fallback).exists()) {
            userAppDir = fallback;
        }
    }
    auto appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    if (!userAppDir.isEmpty()) {
        appDirs.removeAll(userAppDir);
        appDirs.prepend(userAppDir);
    }
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
    sortBrowserOptionsByDisplayName(options);
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

QString getChromeUserDataDir(const QString &desktopPath) {
    if (!desktopPath.endsWith(QStringLiteral(".app")) || !QDir(desktopPath).exists()) {
        return {};
    }
    return getChromiumConfigDirForBundle(desktopPath);
}
