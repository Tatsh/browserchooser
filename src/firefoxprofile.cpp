#include <QtCore/QFile>
#include <QtCore/QSettings>

#include "firefoxprofile.h"
#include "stringconstants.h"

static const auto kProfilePrefix = QStringLiteral("Profile");

QList<FirefoxProfilePair> getFirefoxProfiles(const QString &configDir) {
    static const auto kFmtProfilesIni = QStringLiteral("%1/profiles.ini");
    auto path = kFmtProfilesIni.arg(configDir);
    if (!QFile::exists(path)) {
        return {};
    }
    QSettings ini(path, QSettings::IniFormat);
    int profileCount = 0;
    QList<FirefoxProfilePair> nonDefaultPairs;
    for (const auto &group : ini.childGroups()) {
        if (!group.startsWith(kProfilePrefix)) {
            continue;
        }
        ++profileCount;
        ini.beginGroup(group);
        auto name = ini.value(kName).toString();
        auto isDefault = ini.value(kDefault).toInt() == 1;
        ini.endGroup();
        if (name.isEmpty() || isDefault) {
            continue;
        }
        nonDefaultPairs.append({name, name});
    }
    if (profileCount <= 1) {
        return {{QString(), QString()}}; // LCOV_EXCL_LINE
    }
    QList<FirefoxProfilePair> pairs;
    pairs.append({QString(), QString()});
    for (const auto &pair : nonDefaultPairs) {
        pairs.append(pair);
    }
    return pairs;
}
