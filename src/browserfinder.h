#pragma once

#include <QtCore/QList>

#include "desktopentry.h"

/** If entries with NoDisplay=true should be included. */
enum class IncludeNoDisplay { No, Yes };

/**
 * Find available web browsers on the system.
 * @param includeNoDisplay Whether to include entries with NoDisplay=true.
 * @return A list of DesktopEntry objects representing the found web browsers.
 */
[[nodiscard]] QList<DesktopEntry>
getBrowsers(IncludeNoDisplay includeNoDisplay = IncludeNoDisplay::No);
