#pragma once

#include <expected>

#include <QtCore/QString>

/** Parse the given URL and return the domain or an error message. */
[[nodiscard]] std::expected<QString, QString> parseUrl(const QString &url);
