#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMap>
#include <QtCore/QProcess>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "backend.h"
#include "browseroption.h"
#include "desktopentry.h"
#include "firefox_profile.h"

namespace {

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
    if (base == QStringLiteral("chromium-browser")) {
        return QStringLiteral("chromium");
    }
    if (base == QStringLiteral("google-chrome-stable")) {
        return QStringLiteral("google-chrome");
    }
    if (base == QStringLiteral("brave-browser")) {
        return QStringLiteral("BraveSoftware/Brave-Browser");
    }
    if (base == QStringLiteral("brave")) {
        return QStringLiteral("BraveSoftware/Brave-Browser");
    }
    if (base == QStringLiteral("microsoft-edge-stable")) {
        return QStringLiteral("microsoft-edge");
    }
    if (base == QStringLiteral("microsoft-edge-beta")) {
        return QStringLiteral("microsoft-edge-beta");
    }
    if (base == QStringLiteral("microsoft-edge-dev")) {
        return QStringLiteral("microsoft-edge-dev");
    }
    return base;
}

using ProfilePair = QPair<QString, QString>;

QList<ProfilePair> getChromeProfiles(const QString &configDir) {
    QFile file(configDir + QStringLiteral("/Local State"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return {};
    }
    auto root = doc.object();
    auto profile = root.value(QStringLiteral("profile")).toObject();
    auto infoCache = profile.value(QStringLiteral("info_cache")).toObject();
    QList<ProfilePair> pairs;
    for (auto it = infoCache.begin(); it != infoCache.end(); ++it) {
        auto id = it.key();
        if (id == QStringLiteral("System Profile") || id == QStringLiteral("Guest Profile")) {
            continue;
        }
        auto displayName = it.value().toObject().value(QStringLiteral("name")).toString().trimmed();
        if (displayName.isEmpty()) {
            displayName = id;
        }
        auto launchId = (id == QStringLiteral("Default")) ? QString() : id;
        pairs.append({launchId, displayName});
    }
    return pairs;
}

bool isFirefoxExecutable(const QString &exeName) {
    return exeName.contains(QStringLiteral("firefox"), Qt::CaseInsensitive);
}

// Reads a comma-separated list from the config file (used for Advanced/hideProfileBrowsers
// and Advanced/hideBrowsers). Identifiers can be executable names (e.g. firefox) or full canonical paths.
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
           arg.startsWith(QLatin1String("--profile="))
           || arg.startsWith(QLatin1String("--profile-directory="));
}

QString quoteArg(const QString &arg) {
    if (arg.isEmpty()) {
        return QStringLiteral(R"("")");
    }
    auto needQuote = false;
    for (auto c : arg) {
        if (c == QLatin1Char(' ') || c == QLatin1Char('"') || c == QLatin1Char('\'') ||
            c == QLatin1Char('\\')) {
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
    for (auto c : arg) {
        if (c == QLatin1Char('"')) {
            result += QStringLiteral(R"(\")");
        } else if (c == QLatin1Char('\\')) {
            result += QStringLiteral(R"(\\)");
        } else {
            result += c;
        }
    }
    result += QLatin1Char('"');
    return result;
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
    auto uIndex = argv.indexOf(QStringLiteral("%U"));
    auto uLowerIndex = argv.indexOf(QStringLiteral("%u"));
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
        if (exeName.contains(QStringLiteral("firefox"), Qt::CaseInsensitive)) {
            args.prepend(option.profileName());
            args.prepend(QStringLiteral("-P"));
        } else if (option.profileName() == QStringLiteral("Guest")) {
            args.prepend(QStringLiteral("--guest"));
        } else {
            args.prepend(QStringLiteral("--profile-directory=") + option.profileName());
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
        readCommaSeparatedList(QStringLiteral("Advanced/hideProfileBrowsers"));
    const auto userAppDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    auto appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    if (!userAppDir.isEmpty() && appDirs.removeAll(userAppDir)) {
        appDirs.prepend(userAppDir);
    }
    QList<BrowserOption> options;
    QSet<QString> seenKeys;
    for (const auto &appDir : appDirs) {
        QDir dir(appDir);
        if (!dir.exists()) {
            continue;
        }
        auto desktopFiles = dir.entryList({QStringLiteral("*.desktop")}, QDir::Files);
        for (const auto &desktopFile : desktopFiles) {
            auto fullPath = dir.absoluteFilePath(desktopFile);
            auto entryOpt = readDesktopEntry(fullPath);
            if (!entryOpt.has_value()) {
                continue;
            }
            auto entry = entryOpt.value();
            if (entry.startupWMClass() == QStringLiteral("browserchooser")) {
                continue;
            }
            if (includeNoDisplay == IncludeNoDisplay::No && entry.noDisplay()) {
                continue;
            }
            auto categories = entry.categories();
            auto mimeTypes = entry.mimeTypes();
            auto isWebBrowser = categories.contains(QStringLiteral("WebBrowser"));
            auto handlesHttp = mimeTypes.contains(QStringLiteral("x-scheme-handler/http")) ||
                               mimeTypes.contains(QStringLiteral("x-scheme-handler/https"));
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
                auto configDir = QDir::homePath() + QStringLiteral("/.mozilla/firefox");
                if (QDir(configDir).exists()) {
                    for (const auto &p : getFirefoxProfiles(configDir)) {
                        profilePairs.append(p);
                    }
                    fromProfileDiscovery = !profilePairs.isEmpty();
                }
            } else {
                auto configDirName = getChromiumConfigDirName(exeName);
                auto configDir = getChromeConfigBasePath() + QLatin1Char('/') + configDirName;
                auto localStatePath = configDir + QStringLiteral("/Local State");
                if (QDir(configDir).exists() && QFile::exists(localStatePath)) {
                    profilePairs = getChromeProfiles(configDir);
                    fromProfileDiscovery = !profilePairs.isEmpty();
                }
                if (profilePairs.isEmpty()) {
                    profilePairs.append({QString(), QString()});
                }
                profilePairs.append({QStringLiteral("Guest"), QStringLiteral("Guest")});
            }
            if (profilePairs.isEmpty()) {
                profilePairs.append({QString(), QString()});
            }
            auto singleProfile = profilePairs.size() == 1;
            for (const auto &pair : profilePairs) {
                auto key = fullPath + QLatin1Char('|') + pair.first;
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
    std::ranges::sort(options, [](const BrowserOption &a, const BrowserOption &b) {
        return a.displayName().compare(b.displayName(), Qt::CaseInsensitive) < 0;
    });
    const auto hideBrowsers = readCommaSeparatedList(QStringLiteral("Advanced/hideBrowsers"));
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
    auto entry = option.entry();
    if (!entry.isValid()) {
        return;
    }
    auto argv = parseExecString(entry.exec());
    if (argv.isEmpty()) {
        return;
    }
    QList<QStringList> commands;
    auto uIndex = argv.indexOf(QStringLiteral("%U"));
    auto uLowerIndex = argv.indexOf(QStringLiteral("%u"));
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
            if (exeName.contains(QStringLiteral("firefox"), Qt::CaseInsensitive)) {
                args.prepend(option.profileName());
                args.prepend(QStringLiteral("-P"));
            } else if (option.profileName() == QStringLiteral("Guest")) {
                args.prepend(QStringLiteral("--guest"));
            } else {
                args.prepend(QStringLiteral("--profile-directory=") + option.profileName());
            }
        }
        QProcess::startDetached(program, args);
    }
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

QString getConfigFilePath() {
    auto configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + QStringLiteral("/browserchooserrc");
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
    return getChromeConfigBasePath() + QLatin1Char('/') + configDirName;
}
