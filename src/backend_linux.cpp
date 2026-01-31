#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMap>
#include <QtCore/QProcess>
#include <QtCore/QSet>
#include <QtCore/QStandardPaths>

#include "backend.h"
#include "browseroption.h"
#include "desktopentry.h"
#include "firefox_profile.h"
#include "string_constants.h"

namespace {

static const auto kFmtMozillaFirefox = QStringLiteral("%1/.mozilla/firefox");
static const auto kFmtKey = QStringLiteral("%1|%2");
static const auto kArgGuest = QStringLiteral("--guest");
static const auto kGuestProfile = QStringLiteral("Guest Profile");
static const auto kSystemProfile = QStringLiteral("System Profile");
static const auto kChromiumBrowser = QStringLiteral("chromium-browser");
static const auto kChromium = QStringLiteral("chromium");
static const auto kGoogleChromeStable = QStringLiteral("google-chrome-stable");
static const auto kGoogleChrome = QStringLiteral("google-chrome");
static const auto kBraveBrowser = QStringLiteral("brave-browser");
static const auto kBrave = QStringLiteral("brave");
static const auto kBraveSoftwareBraveBrowser = QStringLiteral("BraveSoftware/Brave-Browser");
static const auto kMicrosoftEdgeStable = QStringLiteral("microsoft-edge-stable");
static const auto kMicrosoftEdge = QStringLiteral("microsoft-edge");
static const auto kMicrosoftEdgeBeta = QStringLiteral("microsoft-edge-beta");
static const auto kMicrosoftEdgeDev = QStringLiteral("microsoft-edge-dev");
static const auto kJsonProfile = QStringLiteral("profile");
static const auto kJsonInfoCache = QStringLiteral("info_cache");
static const auto kJsonName = QStringLiteral("name");
static const auto kPercentU = QStringLiteral("%U");
static const auto kPercentu = QStringLiteral("%u");
static const auto kDesktopGlob = QStringLiteral("*.desktop");
static const auto kBrowserchooser = QStringLiteral("browserchooser");
static const auto kWebBrowser = QStringLiteral("WebBrowser");
static const auto kSchemeHandlerHttp = QStringLiteral("x-scheme-handler/http");
static const auto kSchemeHandlerHttps = QStringLiteral("x-scheme-handler/https");

QString getChromeConfigBasePath() {
    const auto env = qEnvironmentVariable("CHROME_CONFIG_HOME");
    if (!env.isEmpty() && QDir(env).exists()) {
        return env;
    }
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
}

QString getChromiumConfigDirName(const QString &exeName) {
    auto base = QFileInfo(exeName).fileName();
    if (base.isEmpty()) {
        base = exeName;
    }
    if (base == kChromiumBrowser) {
        return kChromium;
    }
    if (base == kGoogleChromeStable) {
        return kGoogleChrome;
    }
    if (base == kBraveBrowser) {
        return kBraveSoftwareBraveBrowser;
    }
    if (base == kBrave) {
        return kBraveSoftwareBraveBrowser;
    }
    if (base == kMicrosoftEdgeStable) {
        return kMicrosoftEdge;
    }
    if (base == kMicrosoftEdgeBeta) {
        return kMicrosoftEdgeBeta;
    }
    if (base == kMicrosoftEdgeDev) {
        return kMicrosoftEdgeDev;
    }
    return base;
}

using ProfilePair = QPair<QString, QString>;

QList<ProfilePair> getChromeProfiles(const QString &configDir) {
    QFile file(kFmtLocalState.arg(configDir));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return {};
    }
    auto root = doc.object();
    auto profile = root.value(kJsonProfile).toObject();
    auto infoCache = profile.value(kJsonInfoCache).toObject();
    QList<ProfilePair> pairs;
    for (auto it = infoCache.begin(); it != infoCache.end(); ++it) {
        auto id = it.key();
        if (id == kSystemProfile || id == kGuestProfile) {
            continue;
        }
        auto displayName = it.value().toObject().value(kJsonName).toString().trimmed();
        if (displayName.isEmpty()) {
            displayName = id;
        }
        auto launchId = (id == kDefault) ? QString() : id;
        pairs.append({launchId, displayName});
    }
    return pairs;
}

bool isFirefoxExecutable(const QString &exeName) {
    return exeName.contains(kFirefox, Qt::CaseInsensitive);
}

QString getCanonicalBrowserPath(const BrowserOption &option) {
    const auto exeName = option.entry().executableName();
    if (exeName.isEmpty()) {
        return {};
    }
    auto exePath = QStandardPaths::findExecutable(exeName);
    if (exePath.isEmpty() && QFileInfo(exeName).isAbsolute() && QFile::exists(exeName)) {
        exePath = exeName;
    }
    if (exePath.isEmpty()) {
        return {};
    }
    const auto canonical = QFileInfo(exePath).canonicalFilePath();
    return canonical.isEmpty() ? exePath : canonical;
}

QStringList parseExecString(const QString &exec) {
    QStringList result;
    QString current;
    auto inQuote = false;
    QChar quoteChar;
    for (auto i = 0; i < exec.length(); ++i) {
        auto c = exec[i];
        if (!inQuote && (c == QLatin1Char('"') || c == QLatin1Char('\''))) {
            inQuote = true;
            quoteChar = c;
        } else if (inQuote && c == quoteChar) {
            inQuote = false;
        } else if (!inQuote && c == QLatin1Char(' ')) {
            if (!current.isEmpty()) {
                result.append(current);
                current.clear();
            }
        } else {
            current.append(c);
        }
    }
    if (!current.isEmpty()) {
        result.append(current);
    }
    return result;
}

bool isProfileRelatedArg(const QString &arg) {
    return arg == QLatin1String("-P") || arg == QLatin1String("--guest") ||
           arg.startsWith(QLatin1String("--profile=")) ||
           arg.startsWith(QLatin1String("--profile-directory="));
}

QStringList buildArgvForOption(const BrowserOption &option, const QString &url) {
    auto entry = option.entry();
    if (!entry.isValid()) {
        return {};
    }
    auto argv = parseExecString(entry.exec());
    if (argv.isEmpty()) {
        return {};
    }
    QStringList cmd;
    auto uIndex = argv.indexOf(kPercentU);
    auto uLowerIndex = argv.indexOf(kPercentu);
    if (uIndex != -1) {
        for (auto i = 0; i < argv.size(); ++i) {
            if (i == uIndex) {
                if (!url.isEmpty()) {
                    cmd.append(url);
                }
            } else if (argv[i] == QLatin1String("-P")) {
                ++i;
            } else if (!isProfileRelatedArg(argv[i])) {
                cmd.append(argv[i]);
            }
        }
    } else if (uLowerIndex != -1) {
        for (auto i = 0; i < argv.size(); ++i) {
            if (i == uLowerIndex) {
                continue;
            }
            if (argv[i] == QLatin1String("-P")) {
                ++i;
                continue;
            }
            if (!isProfileRelatedArg(argv[i])) {
                cmd.append(argv[i]);
            }
        }
        if (!url.isEmpty()) {
            cmd.append(url);
        }
    } else {
        for (auto i = 0; i < argv.size(); ++i) {
            if (argv[i] == QLatin1String("-P")) {
                ++i;
                continue;
            }
            if (isProfileRelatedArg(argv[i]) || argv[i].startsWith(QLatin1Char('%'))) {
                continue;
            }
            cmd.append(argv[i]);
        }
        if (!url.isEmpty()) {
            cmd.append(url);
        }
    }
    if (cmd.isEmpty()) {
        return {};
    }
    auto program = cmd.first();
    auto args = cmd.mid(1);
    if (!option.profileName().isEmpty()) {
        auto exeName = option.entry().executableName();
        if (exeName.contains(kFirefox, Qt::CaseInsensitive)) {
            args.prepend(option.profileName());
            args.prepend(kArgP);
        } else if (option.profileName() == kGuest) {
            args.prepend(kArgGuest);
        } else {
            args.prepend(kFmtProfileDirectory.arg(option.profileName()));
        }
    }
    QStringList result;
    result.append(program);
    result.append(args);
    return result;
}

} // anonymous namespace

QList<BrowserOption> getBrowsers(IncludeNoDisplay includeNoDisplay) {
    const auto hideProfileBrowsers =
        readCommaSeparatedList(kKeyHideProfileBrowsers);
    const auto userAppDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    auto appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    if (!userAppDir.isEmpty()) {
        appDirs.removeAll(userAppDir);
        appDirs.prepend(userAppDir);
    }
    QList<BrowserOption> options;
    QSet<QString> seenKeys;
    for (const auto &appDir : appDirs) {
        QDir dir(appDir);
        if (!dir.exists()) {
            continue;
        }
        auto desktopFiles = dir.entryList({kDesktopGlob}, QDir::Files);
        for (const auto &desktopFile : desktopFiles) {
            auto fullPath = dir.absoluteFilePath(desktopFile);
            auto entryOpt = readDesktopEntry(fullPath);
            if (!entryOpt.has_value()) {
                continue;
            }
            auto entry = entryOpt.value();
            if (entry.startupWMClass() == kBrowserchooser) {
                continue;
            }
            if (includeNoDisplay == IncludeNoDisplay::No && entry.noDisplay()) {
                continue;
            }
            auto categories = entry.categories();
            auto mimeTypes = entry.mimeTypes();
            auto isWebBrowser = categories.contains(kWebBrowser);
            auto handlesHttp = mimeTypes.contains(kSchemeHandlerHttp) ||
                               mimeTypes.contains(kSchemeHandlerHttps);
            if (!isWebBrowser || !handlesHttp) {
                continue;
            }
            auto exeName = entry.executableName();
            if (exeName.isEmpty()) {
                continue;
            }
            auto exePath = QStandardPaths::findExecutable(exeName);
            if (exePath.isEmpty()) {
                if (QFileInfo(exeName).isAbsolute() && QFile::exists(exeName)) {
                    exePath = exeName;
                } else {
                    continue;
                }
            }
            const auto canonicalExePath = QFileInfo(exePath).canonicalFilePath();
            const auto resolvedExePath = canonicalExePath.isEmpty() ? exePath : canonicalExePath;
            const bool skipProfileDiscovery =
                listContainsIdentifier(hideProfileBrowsers, exeName) ||
                listContainsIdentifier(hideProfileBrowsers, resolvedExePath);
            QList<ProfilePair> profilePairs;
            auto fromProfileDiscovery = false;
            if (skipProfileDiscovery) {
                profilePairs.append({QString(), QString()});
            } else if (isFirefoxExecutable(exeName)) {
                auto configDir = kFmtMozillaFirefox.arg(QDir::homePath());
                if (QDir(configDir).exists()) {
                    for (const auto &p : getFirefoxProfiles(configDir)) {
                        profilePairs.append(p);
                    }
                    fromProfileDiscovery = !profilePairs.isEmpty();
                }
            } else {
                auto configDirName = getChromiumConfigDirName(exeName);
                auto configDir =
                    kFmtPath.arg(getChromeConfigBasePath(), configDirName);
                auto localStatePath = kFmtLocalState.arg(configDir);
                if (QDir(configDir).exists() && QFile::exists(localStatePath)) {
                    profilePairs = getChromeProfiles(configDir);
                    fromProfileDiscovery = !profilePairs.isEmpty();
                }
                if (profilePairs.isEmpty()) {
                    profilePairs.append({QString(), QString()});
                }
                profilePairs.append({kGuest, kGuest});
            }
            if (profilePairs.isEmpty()) {
                profilePairs.append({QString(), QString()});
            }
            auto singleProfile = profilePairs.size() == 1;
            for (const auto &pair : profilePairs) {
                auto key = kFmtKey.arg(fullPath, pair.first);
                if (seenKeys.contains(key)) {
                    continue;
                }
                seenKeys.insert(key);
                options.append(BrowserOption(
                    entry, pair.first, pair.second, singleProfile, fromProfileDiscovery));
            }
        }
    }
    const auto userAppDirForPref =
        QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    auto isUserDesktop = [&userAppDirForPref](const QString &desktopPath) {
        return !userAppDirForPref.isEmpty() && desktopPath.startsWith(userAppDirForPref);
    };
    QMap<QString, BrowserOption> byDisplayName;
    for (const auto &opt : options) {
        auto name = opt.displayName();
        auto it = byDisplayName.find(name);
        if (it == byDisplayName.end()) {
            byDisplayName.insert(name, opt);
        } else {
            const auto &existing = it.value();
            auto preferNew = false;
            if (isUserDesktop(opt.desktopPath()) && !isUserDesktop(existing.desktopPath())) {
                preferNew = true;
            } else if (isUserDesktop(opt.desktopPath()) == isUserDesktop(existing.desktopPath()) &&
                       opt.fromProfileDiscovery() && !existing.fromProfileDiscovery()) {
                preferNew = true;
            }
            if (preferNew) {
                it.value() = opt;
            }
        }
    }
    options = byDisplayName.values();
    sortBrowserOptionsByDisplayName(options);
    const auto hideBrowsers = readCommaSeparatedList(kKeyHideBrowsers);
    if (!hideBrowsers.isEmpty()) {
        options.removeIf([&hideBrowsers](const BrowserOption &opt) {
            const auto exeName = opt.entry().executableName();
            const auto canonicalPath = getCanonicalBrowserPath(opt);
            return listContainsIdentifier(hideBrowsers, exeName) ||
                   listContainsIdentifier(hideBrowsers, canonicalPath);
        });
    }
    return options;
}

void launchBrowser(const BrowserOption &option, const QStringList &urls) {
    runPreLaunchCommands(option.desktopPath(), option.profileName());
    auto entry = option.entry();
    if (!entry.isValid()) {
        return;
    }
    auto argv = parseExecString(entry.exec());
    if (argv.isEmpty()) {
        return;
    }
    QList<QStringList> commands;
    auto uIndex = argv.indexOf(kPercentU);
    auto uLowerIndex = argv.indexOf(kPercentu);
    if (uIndex != -1) {
        QStringList cmd;
        for (auto i = 0; i < argv.size(); ++i) {
            if (i == uIndex) {
                cmd.append(urls);
            } else if (argv[i] == QLatin1String("-P")) {
                ++i;
            } else if (!isProfileRelatedArg(argv[i])) {
                cmd.append(argv[i]);
            }
        }
        commands.append(cmd);
    } else if (uLowerIndex != -1) {
        if (urls.isEmpty()) {
            QStringList cmd;
            for (auto i = 0; i < argv.size(); ++i) {
                if (i == uLowerIndex) {
                    continue;
                }
                if (argv[i] == QLatin1String("-P")) {
                    ++i;
                    continue;
                }
                if (!isProfileRelatedArg(argv[i])) {
                    cmd.append(argv[i]);
                }
            }
            commands.append(cmd);
        } else {
            for (const auto &url : urls) {
                QStringList cmd;
                for (auto i = 0; i < argv.size(); ++i) {
                    if (i == uLowerIndex) {
                        cmd.append(url);
                    } else if (argv[i] == QLatin1String("-P")) {
                        ++i;
                        continue;
                    } else if (!isProfileRelatedArg(argv[i])) {
                        cmd.append(argv[i]);
                    }
                }
                commands.append(cmd);
            }
        }
    } else {
        QStringList cmd;
        for (auto i = 0; i < argv.size(); ++i) {
            if (argv[i] == QLatin1String("-P")) {
                ++i;
                continue;
            }
            if (isProfileRelatedArg(argv[i]) || argv[i].startsWith(QLatin1Char('%'))) {
                continue;
            }
            cmd.append(argv[i]);
        }
        if (!urls.isEmpty()) {
            cmd.append(urls);
        }
        commands.append(cmd);
    }
    for (auto cmd : commands) {
        if (cmd.isEmpty()) {
            continue;
        }
        auto program = cmd.first();
        auto args = cmd.mid(1);
        if (!option.profileName().isEmpty()) {
            auto exeName = option.entry().executableName();
            if (exeName.contains(kFirefox, Qt::CaseInsensitive)) {
                args.prepend(option.profileName());
                args.prepend(kArgP);
            } else if (option.profileName() == kGuest) {
                args.prepend(kArgGuest);
            } else {
                args.prepend(kFmtProfileDirectory.arg(option.profileName()));
            }
        }
        QProcess::startDetached(program, args);
    }
    runPostLaunchCommands(option.desktopPath(), option.profileName());
}

QString getCommandLineForDisplay(const BrowserOption &option, const QString &url) {
    auto argv = buildArgvForOption(option, url);
    if (argv.isEmpty()) {
        return {};
    }
    auto program = argv.first();
    auto fullPath = QStandardPaths::findExecutable(program);
    if (!fullPath.isEmpty()) {
        argv[0] = fullPath;
    }
    QStringList quoted;
    for (const auto &arg : argv) {
        quoted.append(quoteArg(arg));
    }
    return quoted.join(QLatin1Char(' '));
}

QString getExecutablePath(const BrowserOption &option) {
    auto entry = option.entry();
    if (!entry.isValid()) {
        return {};
    }
    auto exeName = entry.executableName();
    if (exeName.isEmpty()) {
        return {};
    }
    auto exePath = QStandardPaths::findExecutable(exeName);
    if (exePath.isEmpty() && QFileInfo(exeName).isAbsolute() && QFile::exists(exeName)) {
        exePath = exeName;
    }
    return exePath;
}

QString getChromeUserDataDir(const QString &desktopPath) {
    auto entryOpt = readDesktopEntry(desktopPath);
    if (!entryOpt.has_value()) {
        return {};
    }
    auto exeName = entryOpt->executableName();
    if (isFirefoxExecutable(exeName)) {
        return {};
    }
    auto configDirName = getChromiumConfigDirName(exeName);
    return kFmtPath.arg(getChromeConfigBasePath(), configDirName);
}
