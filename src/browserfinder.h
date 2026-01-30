/** @file */
#pragma once

#include <QtCore/QList>

#include "browseroption.h"

/** If entries with NoDisplay=true should be included. */
enum class IncludeNoDisplay { No, Yes };

/**
 * Finds available web browsers on the system.
 * Only includes browsers whose executable is in @c PATH.
 * Expands Chrome/Chromium entries into one option per profile (Default + named).
 * @param includeNoDisplay Whether to include entries with @c NoDisplay=true.
 * @return List of BrowserOption objects (browser + optional profile).
 */
[[nodiscard]] QList<BrowserOption>
getBrowsers(IncludeNoDisplay includeNoDisplay = IncludeNoDisplay::No);

/**
 * Resolves the profile display name for a Chromium-based browser (for saved options).
 * @param desktopPath Path to the @c .desktop file.
 * @param profileId Profile identifier (e.g. "Default", "Profile 1").
 * @return Display name for the profile, or empty if not Chromium or profile not found.
 */
[[nodiscard]] QString getChromeProfileDisplayName(const QString &desktopPath,
                                                  const QString &profileId);
