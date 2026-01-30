#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "chrome_profile.h"

QString getChromeProfileDisplayNameFromUserDataDir(const QString &userDataDir,
                                                   const QString &profileId) {
    QFile file(userDataDir + QStringLiteral("/Local State"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return {};
    }
    const auto root = doc.object();
    const auto profile = root.value(QStringLiteral("profile")).toObject();
    const auto infoCache = profile.value(QStringLiteral("info_cache")).toObject();
    const auto key = profileId.isEmpty() ? QStringLiteral("Default") : profileId;
    if (!infoCache.contains(key)) {
        return {};
    }
    auto name = infoCache.value(key).toObject().value(QStringLiteral("name")).toString().trimmed();
    if (name.isEmpty()) {
        name = key;
    }
    return name;
}
