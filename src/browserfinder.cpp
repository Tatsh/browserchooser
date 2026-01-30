#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "browserfinder.h"
#include "browseroption.h"
#include "desktopentry.h"

namespace {

QString getChromiumConfigDirName(const QString &exeName) {
    auto base = QFileInfo(exeName).fileName();
    if (base.isEmpty()) {
        base = exeName;
    }
    // Known mappings where package/executable name differs from config dir.
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

using ProfilePair = QPair<QString, QString>; // (profile id, display name)

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
        // Use empty id for Default so we don't pass --profile= to launch.
        auto launchId = (id == QStringLiteral("Default")) ? QString() : id;
        pairs.append({launchId, displayName});
    }
    return pairs;
}

bool isFirefoxExecutable(const QString &exeName) {
    return exeName.contains(QStringLiteral("firefox"), Qt::CaseInsensitive);
}

QList<ProfilePair> getFirefoxProfiles(const QString &configDir) {
    auto path = configDir + QStringLiteral("/profiles.ini");
    if (!QFile::exists(path)) {
        return {};
    }
    QSettings ini(path, QSettings::IniFormat);
    QList<ProfilePair> pairs;
    pairs.append({QString(), QString()}); // Default (no -P), display "Default".
    for (const auto &group : ini.childGroups()) {
        if (!group.startsWith(QStringLiteral("Profile"))) {
            continue;
        }
        ini.beginGroup(group);
        auto name = ini.value(QStringLiteral("Name")).toString();
        auto isDefault = ini.value(QStringLiteral("Default")).toInt() == 1;
        ini.endGroup();
        if (name.isEmpty() || isDefault) {
            continue;
        }
        pairs.append({name, name}); // Firefox uses Name for both -P and display.
    }
    return pairs;
}

} // anonymous namespace

QList<BrowserOption> getBrowsers(IncludeNoDisplay includeNoDisplay) {
    const auto userAppDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    auto appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    // Prefer user directory (~/.local/share/applications) over system
    if (!userAppDir.isEmpty() && appDirs.removeAll(userAppDir)) {
        appDirs.prepend(userAppDir);
    }
    QList<BrowserOption> options;
    QSet<QString> seenKeys; // Dedupe by (desktop path, profile).
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
            // Only offer if the executable is in PATH or is an absolute path that exists.
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
            QList<ProfilePair> profilePairs;
            auto fromProfileDiscovery = false;
            if (isFirefoxExecutable(exeName)) {
                auto configDir = QDir::homePath() + QStringLiteral("/.mozilla/firefox");
                if (QDir(configDir).exists()) {
                    profilePairs = getFirefoxProfiles(configDir);
                    fromProfileDiscovery = !profilePairs.isEmpty();
                }
            } else {
                auto configDirName = getChromiumConfigDirName(exeName);
                auto configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
                                 QLatin1Char('/') + configDirName;
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
    // De-duplicate by display name; prefer user .desktop over system, then profile discovery.
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
    return options;
}

QString getChromeProfileDisplayName(const QString &desktopPath, const QString &profileId) {
    if (profileId == QStringLiteral("Guest")) {
        return QStringLiteral("Guest");
    }
    auto entryOpt = readDesktopEntry(desktopPath);
    if (!entryOpt.has_value()) {
        return {};
    }
    auto exeName = entryOpt->executableName();
    if (isFirefoxExecutable(exeName)) {
        return {};
    }
    auto configDirName = getChromiumConfigDirName(exeName);
    auto configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
                     QLatin1Char('/') + configDirName;
    for (const auto &pair : getChromeProfiles(configDir)) {
        if (pair.first == profileId) {
            return pair.second;
        }
    }
    return {};
}
