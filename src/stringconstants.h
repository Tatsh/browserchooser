/** @file */
#pragma once

#include <QtCore/QString>

/** Command line argument for specifying a profile name for Firefox. */
inline const QString kArgP = QStringLiteral("-P");
/** Default profile name. */
inline const QString kDefault = QStringLiteral("Default");
/** Firefox binary name. */
inline const QString kFirefox = QStringLiteral("firefox");
/** Format string for Chromium's Local State file path. */
inline const QString kFmtLocalState = QStringLiteral("%1/Local State");
/** Format string for constructing paths. */
inline const QString kFmtPath = QStringLiteral("%1/%2");
/** Chromium profile directory argument format. */
inline const QString kFmtProfileDirectory = QStringLiteral("--profile-directory=%1");
/** Used for guest profiles. */
inline const QString kGuest = QStringLiteral("Guest");
/** Generic "Name" key. */
inline const QString kName = QStringLiteral("Name");
/** Settings key for hiding browsers. */
inline const QString kKeyHideBrowsers = QStringLiteral("Advanced/hideBrowsers");
/** Settings key for hiding profile browsers. */
inline const QString kKeyHideProfileBrowsers = QStringLiteral("Advanced/hideProfileBrowsers");
