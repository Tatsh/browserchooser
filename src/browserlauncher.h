/** @file */
#pragma once

#include <QtCore/QStringList>

#include "browseroption.h"
#include "desktopentry.h"

/**
 * Launches the given browser option with the specified URLs.
 * For Chrome/Chromium profiles, adds @c --profile= when not Default.
 * @param option The browser and optional profile to launch.
 * @param urls List of URLs to open; defaults to empty.
 */
void launchBrowser(const BrowserOption &option, const QStringList &urls = QStringList());

/**
 * Builds the command line for display (e.g. tooltip), including @c --profile, @c -P, @c --guest
 * and with arguments properly quoted.
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
