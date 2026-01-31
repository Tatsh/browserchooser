#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "chrome_profile.h"
#include "string_constants.h"

static const auto kFmtProfilePath = QStringLiteral("%1/%2/%3");
static const auto kJsonProfile = QStringLiteral("profile");
static const auto kJsonInfoCache = QStringLiteral("info_cache");
static const auto kJsonName = QStringLiteral("name");
static const auto kJsonGaiaPictureFileName = QStringLiteral("gaia_picture_file_name");

QString getChromeProfileDisplayNameFromUserDataDir(const QString &userDataDir,
                                                   const QString &profileId) {
    QFile file(kFmtLocalState.arg(userDataDir));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return {};
    }
    const auto root = doc.object();
    const auto profile = root.value(kJsonProfile).toObject();
    const auto infoCache = profile.value(kJsonInfoCache).toObject();
    const auto key = profileId.isEmpty() ? kDefault : profileId;
    if (!infoCache.contains(key)) {
        return {};
    }
    auto name = infoCache.value(key).toObject().value(kJsonName).toString().trimmed();
    if (name.isEmpty()) {
        name = key;
    }
    return name;
}

QString getChromeProfilePicturePathFromUserDataDir(const QString &userDataDir,
                                                   const QString &profileId) {
    QFile file(kFmtLocalState.arg(userDataDir));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return {};
    }
    const auto root = doc.object();
    const auto profile = root.value(kJsonProfile).toObject();
    const auto infoCache = profile.value(kJsonInfoCache).toObject();
    const auto key = profileId.isEmpty() ? kDefault : profileId;
    if (!infoCache.contains(key)) {
        return {};
    }
    const auto fileName =
        infoCache.value(key).toObject().value(kJsonGaiaPictureFileName).toString();
    if (fileName.isEmpty()) {
        return {};
    }
    const auto path = kFmtProfilePath.arg(userDataDir, key, fileName);
    return QFile::exists(path) ? path : QString();
}
