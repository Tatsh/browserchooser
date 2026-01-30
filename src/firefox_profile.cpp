#include <QtCore/QFile>
#include <QtCore/QSettings>

#include "firefox_profile.h"

QList<FirefoxProfilePair> getFirefoxProfiles(const QString &configDir) {
    auto path = configDir + QStringLiteral("/profiles.ini");
    if (!QFile::exists(path)) {
        return {};
    }
    QSettings ini(path, QSettings::IniFormat);
    int profileCount = 0;
    QList<FirefoxProfilePair> nonDefaultPairs;
    for (const auto &group : ini.childGroups()) {
        if (!group.startsWith(QStringLiteral("Profile"))) {
            continue;
        }
        ++profileCount;
        ini.beginGroup(group);
        auto name = ini.value(QStringLiteral("Name")).toString();
        auto isDefault = ini.value(QStringLiteral("Default")).toInt() == 1;
        ini.endGroup();
        if (name.isEmpty() || isDefault) {
            continue;
        }
        nonDefaultPairs.append({name, name});
    }
    if (profileCount <= 1) {
        return {{QString(), QString()}};
    }
    QList<FirefoxProfilePair> pairs;
    pairs.append({QString(), QString()});
    for (const auto &pair : nonDefaultPairs) {
        pairs.append(pair);
    }
    return pairs;
}
