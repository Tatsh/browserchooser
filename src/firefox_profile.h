/** @file */
#pragma once

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>

/** Pair of (profile ID for launch, display name for UI). */
using FirefoxProfilePair = QPair<QString, QString>;

/**
 * Reads the Firefox profiles.ini in the given config directory and returns
 * profile pairs. Same format on Linux, macOS, and Windows.
 * @param configDir Path to the Firefox config dir (contains profiles.ini).
 * @return List of (launch ID, display name) pairs; single profile yields one default entry.
 */
[[nodiscard]] QList<FirefoxProfilePair> getFirefoxProfiles(const QString &configDir);
