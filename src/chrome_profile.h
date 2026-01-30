/** @file */
#pragma once

#include <QtCore/QString>

/**
 * Reads the Chrome/Chromium user data dir "Local State" and returns the profile
 * display name for the given profile ID. Same JSON structure on all platforms.
 * @param userDataDir Path to the user data dir (contains "Local State").
 * @param profileId Profile ID (e.g. "Default", "Profile 1"); empty means Default.
 * @return Display name for the profile, or empty if not found or file invalid.
 */
[[nodiscard]] QString getChromeProfileDisplayNameFromUserDataDir(
    const QString &userDataDir, const QString &profileId);
