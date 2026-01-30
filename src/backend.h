/** @file */
#pragma once

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "browseroption.h"

/** If entries with NoDisplay=true should be included (Linux XDG). */
enum class IncludeNoDisplay { No, Yes };

/**
 * Finds available web browsers on the current platform.
 * @param includeNoDisplay Whether to include entries with @c NoDisplay=true (Linux only).
 * @return List of BrowserOption objects.
 */
[[nodiscard]] QList<BrowserOption>
getBrowsers(IncludeNoDisplay includeNoDisplay = IncludeNoDisplay::No);

/**
 * Launches the given browser option with the specified URLs.
 * @param option The browser and optional profile to launch.
 * @param urls List of URLs to open; defaults to empty.
 */
void launchBrowser(const BrowserOption &option, const QStringList &urls = QStringList());

/**
 * Builds the command line for display (e.g. tooltip).
 * @param option The browser option.
 * @param url Single URL to substitute, or empty.
 * @return The full command line string.
 */
[[nodiscard]] QString getCommandLineForDisplay(const BrowserOption &option,
                                               const QString &url = QString());

/**
 * Returns the resolved full path to the browser executable.
 * @param option The browser option.
 * @return Full path to the executable, or empty if not found.
 */
[[nodiscard]] QString getExecutablePath(const BrowserOption &option);

/**
 * Returns the path to the configuration file for the current platform.
 * @return Full path to the config file.
 */
[[nodiscard]] QString getConfigFilePath();

/**
 * Resolves the profile display name for a saved option (Linux: Chromium profile; other backends may return empty).
 * @param desktopPath Path to the @c .desktop file (Linux) or backend-specific identifier.
 * @param profileId Profile identifier.
 * @return Display name for the profile, or empty if not found or not applicable.
 */
[[nodiscard]] QString getChromeProfileDisplayName(const QString &desktopPath,
                                                  const QString &profileId);

/**
 * Resolves the full path to the Chrome/Chromium profile picture (gaia_picture_file_name in Local State).
 * @param desktopPath Path to the @c .desktop file (Linux) or backend-specific identifier.
 * @param profileId Profile identifier.
 * @return Full path to the picture file, or empty if not found or not applicable.
 */
[[nodiscard]] QString getChromeProfilePicturePath(const QString &desktopPath,
                                                  const QString &profileId);

/**
 * Returns the Chrome/Chromium user data directory for the given browser path.
 * Used by getChromeProfileDisplayName and getChromeProfilePicturePath.
 * @param desktopPath Path to the @c .desktop file (Linux), .app (macOS), or .exe (Windows).
 * @return User data directory path, or empty if not Chrome/Chromium or not found.
 */
[[nodiscard]] QString getChromeUserDataDir(const QString &desktopPath);
