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

/**
 * Reads a comma-separated list from the config file (e.g. Advanced/hideProfileBrowsers).
 * @param key Settings key to read.
 */
[[nodiscard]] QStringList readCommaSeparatedList(const QString &key);

/**
 * Returns true if @p list contains @p identifier (case-insensitive).
 * @param list List of strings to search.
 */
[[nodiscard]] bool listContainsIdentifier(const QStringList &list, const QString &identifier);

/**
 * Quotes @p arg for display in a command line (handles space, quotes, backslash).
 * @param arg Argument to quote.
 */
[[nodiscard]] QString quoteArg(const QString &arg);

/**
 * Sorts browser options by display name (case-insensitive).
 * @param options List of BrowserOption objects to sort.
 */
void sortBrowserOptionsByDisplayName(QList<BrowserOption> &options);

/**
 * Returns the pre-launch commands from config for the given browser or browser+profile.
 * Key is the same as RememberedBrowsers: @p desktopPath, or @p desktopPath + @c "|" + @p profileName.
 * Under group @c PreLaunchCommands. Value is a JSON array of arrays of strings.
 * If no entry for browser+profile, falls back to browser-only (desktopPath).
 * @param desktopPath Path to the .desktop file (Linux) or backend-specific identifier.
 * @param profileName Profile identifier.
 * @return List of command argument lists.
 */
[[nodiscard]] QList<QStringList> getPreLaunchCommands(const QString &desktopPath,
                                                      const QString &profileName);

/**
 * Returns the post-launch commands from config for the given browser or browser+profile.
 * Key is the same as RememberedBrowsers: @p desktopPath, or @p desktopPath + @c "|" + @p profileName.
 * Under group @c PostLaunchCommands. Value is a JSON array of arrays of strings.
 * If no entry for browser+profile, falls back to browser-only (desktopPath).
 * @param desktopPath Path to the .desktop file (Linux) or backend-specific identifier.
 * @param profileName Profile identifier.
 * @return List of command argument lists.
 */
[[nodiscard]] QList<QStringList> getPostLaunchCommands(const QString &desktopPath,
                                                       const QString &profileName);

/**
 * Runs a launch-hook command (pre or post). Executes @p argv directly (no shell).
 * @param argv Program as first element, then arguments. Empty or single empty string is a no-op.
 * @param wait If true, block until the command exits; if false, start detached.
 */
void runLaunchHookCommand(const QStringList &argv, bool wait);

/**
 * Runs all pre-launch commands for the given browser or browser+profile (synchronously).
 * @param desktopPath Path to the .desktop file (Linux) or backend-specific identifier.
 * @param profileName Profile identifier.
 */
void runPreLaunchCommands(const QString &desktopPath, const QString &profileName);

/**
 * Runs all post-launch commands for the given browser or browser+profile (detached).
 * @param desktopPath Path to the .desktop file (Linux) or backend-specific identifier.
 * @param profileName Profile identifier.
 */
void runPostLaunchCommands(const QString &desktopPath, const QString &profileName);
