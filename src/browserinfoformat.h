/** @file */
#pragma once

#include <QtCore/QString>

/**
 * Formats browser command line and desktop path as HTML for display (e.g. tooltip).
 * @param commandLine The command line string.
 * @param desktopPath The desktop file path.
 * @return HTML string with escaped content and expected structure.
 */
[[nodiscard]] QString formatBrowserInfoHtml(const QString &commandLine, const QString &desktopPath);
