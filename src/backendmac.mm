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
#include "firefoxprofile.h"
#include "stringconstants.h"

namespace {

static const auto kFmtPlistPath = QStringLiteral("%1/Contents/Info.plist");
static const auto kFmtMacOSPath = QStringLiteral("%1/Contents/MacOS/%2");
static const auto kSuffixApp = QStringLiteral(".app");
static const auto kSpaceArgs = QStringLiteral(" --args");
static const auto kAppGoogleChrome = QStringLiteral("Google Chrome");
static const auto kDirGoogleChrome = QStringLiteral("Google/Chrome");
static const auto kAppChromium = QStringLiteral("Chromium");
static const auto kAppChrome = QStringLiteral("Chrome");
static const auto kAppBrave = QStringLiteral("Brave");
static const auto kDirBraveSoftware = QStringLiteral("BraveSoftware/Brave-Browser");
static const auto kAppEdge = QStringLiteral("Edge");
static const auto kAppMicrosoftEdge = QStringLiteral("Microsoft Edge");
static const auto kAppFirefox = QStringLiteral("Firefox");
static const auto kAppSafari = QStringLiteral("Safari");
static const auto kArgA = QStringLiteral("-a");
static const auto kArgArgs = QStringLiteral("--args");
static const auto kOpen = QStringLiteral("open");

static NSDictionary *loadPlist(const QString &plistPath) {
    const QByteArray pathUtf8 = plistPath.toUtf8();
    NSString *path = [NSString stringWithUTF8String:pathUtf8.constData()];
    return [NSDictionary dictionaryWithContentsOfFile:path];
}

bool appHandlesHttpHttps(const QString &bundlePath) {
    const auto plistPath = kFmtPlistPath.arg(bundlePath);
    if (!QFile::exists(plistPath)) {
        return false;
    }
    @autoreleasepool {
        NSDictionary *root = loadPlist(plistPath);
        if (!root) {
            return false;
        }
        NSArray *urlTypes = root[@"CFBundleURLTypes"];
        if (!urlTypes || ![urlTypes isKindOfClass:NSArray.class]) {
            return false;
        }
        for (id typeVal in urlTypes) {
            if (![typeVal isKindOfClass:NSDictionary.class]) {
                continue;
            }
            NSDictionary *type = (NSDictionary *)typeVal;
            NSArray *schemes = type[@"CFBundleURLSchemes"];
            if (!schemes || ![schemes isKindOfClass:NSArray.class]) {
                continue;
            }
            for (id schemeVal in schemes) {
                if (![schemeVal isKindOfClass:NSString.class]) {
                    continue;
                }
                NSString *scheme = ((NSString *)schemeVal).lowercaseString;
                if ([scheme isEqualToString:@"http"] || [scheme isEqualToString:@"https"]) {
                    return true;
                }
            }
        }
    }
    return false;
}

QString getChromiumConfigDirForBundle(const QString &bundlePath) {
    const auto plistPath = kFmtPlistPath.arg(bundlePath);
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
        if (crProductDir && [crProductDir isKindOfClass:NSString.class] &&
            crProductDir.length > 0) {
            return kFmtPath.arg(supportBase, QString::fromNSString(crProductDir));
        }
        NSString *appName = root[@"CFBundleName"];
        if (!appName || ![appName isKindOfClass:NSString.class] || appName.length == 0) {
            appName = root[@"CFBundleExecutable"];
        }
        if (!appName || ![appName isKindOfClass:NSString.class] || appName.length == 0) {
            return {};
        }
        auto appNameQ = QString::fromNSString(appName);
        QString dirName;
        if (appNameQ == kAppGoogleChrome) {
            dirName = kDirGoogleChrome;
        } else if (appNameQ == kAppChromium) {
            dirName = kAppChromium;
        } else if (appNameQ.contains(kAppChrome)) {
            static const auto kFmtGoogleDir = QStringLiteral("Google/%1");
            dirName = kFmtGoogleDir.arg(appNameQ);
        } else if (appNameQ.contains(kAppBrave, Qt::CaseInsensitive)) {
            dirName = kDirBraveSoftware;
        } else if (appNameQ.contains(kAppEdge, Qt::CaseInsensitive)) {
            dirName = kAppMicrosoftEdge;
        } else {
            return {};
        }
        return kFmtPath.arg(supportBase, dirName);
    }
}

QString getFirefoxConfigDir() {
    const auto supportBase = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (supportBase.isEmpty()) {
        return {};
    }
    static const auto kFmtFirefox = QStringLiteral("%1/Firefox");
    return kFmtFirefox.arg(supportBase);
}

bool isFirefoxBundle(const QString &bundlePath) {
    const auto baseName = QFileInfo(bundlePath).completeBaseName();
    return baseName.contains(kAppFirefox, Qt::CaseInsensitive);
}

bool isSafariBundle(const QString &bundlePath) {
    const auto baseName = QFileInfo(bundlePath).completeBaseName();
    return baseName.compare(kAppSafari, Qt::CaseInsensitive) == 0;
}

bool isWebBrowser(const QString &bundlePath) {
    return isSafariBundle(bundlePath) || appHandlesHttpHttps(bundlePath);
}

QString getCanonicalBrowserPath(const BrowserOption &option) {
    const auto bundlePath = option.desktopPath();
    if (bundlePath.isEmpty() || !bundlePath.endsWith(kSuffixApp)) {
        return {};
    }
    const auto execName = option.entry().exec();
    if (execName.isEmpty()) {
        return {};
    }
    const auto binaryPath = kFmtMacOSPath.arg(bundlePath, execName);
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
    const auto plistPath = kFmtPlistPath.arg(bundlePath);
    if (!QFile::exists(plistPath)) {
        return {};
    }
    @autoreleasepool {
        NSDictionary *root = loadPlist(plistPath);
        if (!root) {
            return {};
        }
        id value = root[@"CFBundleIdentifier"];
        if (![value isKindOfClass:NSString.class]) {
            return {};
        }
        return QString::fromNSString(value);
    }
}

} // anonymous namespace

QList<BrowserOption> getBrowsers(IncludeNoDisplay) {
    QList<BrowserOption> options;
    // Use ApplicationsLocation for /Applications and ~/Applications on macOS.
    auto userAppDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    if (userAppDir.isEmpty()) {
        static const auto kFmtApplications = QStringLiteral("%1/Applications");
        const auto fallback = kFmtApplications.arg(QDir::homePath());
        if (QDir(fallback).exists()) {
            userAppDir = fallback;
        }
    }
    auto appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    if (!userAppDir.isEmpty()) {
        appDirs.removeAll(userAppDir);
        appDirs.prepend(userAppDir);
    }
    const auto hideProfileBrowsers = readCommaSeparatedList(kKeyHideProfileBrowsers);
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
            if (!name.endsWith(kSuffixApp)) {
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
                entry.exec().isEmpty() ? QString() : kFmtMacOSPath.arg(bundlePath, entry.exec());
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
    const auto hideBrowsers = readCommaSeparatedList(kKeyHideBrowsers);
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
    runPreLaunchCommands(option.desktopPath(), option.profileName());
    const auto bundlePath = option.desktopPath();
    if (!bundlePath.endsWith(kSuffixApp) || !QDir(bundlePath).exists()) {
        return;
    }
    QStringList args{kArgA, bundlePath};
    QStringList appArgs;
    if (!option.profileName().isEmpty()) {
        if (isFirefoxBundle(bundlePath)) {
            appArgs << kArgP << option.profileName();
        } else {
            appArgs << kFmtProfileDirectory.arg(option.profileName());
        }
    } else if (!isFirefoxBundle(bundlePath)) {
        appArgs << kFmtProfileDirectory.arg(kDefault);
    }
    appArgs << urls;
    if (!appArgs.isEmpty()) {
        args << kArgArgs << appArgs;
    }
    QProcess::startDetached(kOpen, args);
    runPostLaunchCommands(option.desktopPath(), option.profileName());
}

QString getCommandLineForDisplay(const BrowserOption &option, const QString &url) {
    static const auto kFmtOpenApp = QStringLiteral("open -a \"%1\"");
    const auto bundlePath = option.desktopPath();
    QString cmd = kFmtOpenApp.arg(bundlePath);
    QStringList appArgs;
    if (!option.profileName().isEmpty()) {
        if (isFirefoxBundle(bundlePath)) {
            appArgs << kArgP << option.profileName();
        } else {
            appArgs << kFmtProfileDirectory.arg(option.profileName());
        }
    } else if (!isFirefoxBundle(bundlePath)) {
        appArgs << kFmtProfileDirectory.arg(kDefault);
    }
    if (!url.isEmpty()) {
        appArgs << url;
    }
    if (!appArgs.isEmpty()) {
        cmd += kSpaceArgs;
        for (const auto &arg : appArgs) {
            static const auto kFmtQuotedArg = QStringLiteral(" \"%1\"");
            cmd += kFmtQuotedArg.arg(arg);
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
    if (!desktopPath.endsWith(kSuffixApp) || !QDir(desktopPath).exists()) {
        return {};
    }
    return getChromiumConfigDirForBundle(desktopPath);
}
