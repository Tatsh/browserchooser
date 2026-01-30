/** @file */
#pragma once

#include <expected>

#include <QtCore/QString>

/** Error code for URL parsing. */
enum class ParseUrlError { EmptyUrl };

/**
 * Parses the given URL and returns the domain (host for http(s), filename for file).
 * @param url The URL or path to parse.
 * @return The extracted domain, or an error if the URL is empty or invalid.
 */
[[nodiscard]] std::expected<QString, ParseUrlError> parseUrl(const QString &url);
