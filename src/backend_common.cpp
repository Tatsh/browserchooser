#include <QtCore/QFile>

#include "backend.h"
#include "chrome_profile.h"

QString getChromeProfileDisplayName(const QString &desktopPath, const QString &profileId) {
    if (profileId == QStringLiteral("Guest")) {
        return QStringLiteral("Guest");
    }
    const auto userDataDir = getChromeUserDataDir(desktopPath);
    if (userDataDir.isEmpty() || !QFile::exists(userDataDir + QStringLiteral("/Local State"))) {
        return {};
    }
    return getChromeProfileDisplayNameFromUserDataDir(userDataDir, profileId);
}

QString getChromeProfilePicturePath(const QString &desktopPath, const QString &profileId) {
    if (profileId == QStringLiteral("Guest")) {
        return {};
    }
    const auto userDataDir = getChromeUserDataDir(desktopPath);
    if (userDataDir.isEmpty() || !QFile::exists(userDataDir + QStringLiteral("/Local State"))) {
        return {};
    }
    return getChromeProfilePicturePathFromUserDataDir(userDataDir, profileId);
}
