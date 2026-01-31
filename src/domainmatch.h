/** @file */
#pragma once

#include <QtCore/QString>

/**
 * Returns true if @p domain matches the wildcard @p pattern.
 * Special handling for *. prefix: matches any subdomain and the bare domain
 * (e.g. *.google.com matches www.google.com, mail.google.com, google.com).
 * Other patterns use QRegularExpression wildcard conversion (case-insensitive).
 * @param pattern The pattern (e.g. *.example.com or *.*.example.com).
 * @param domain The domain to match.
 */
[[nodiscard]] bool matchesWildcardPattern(const QString &pattern, const QString &domain);
