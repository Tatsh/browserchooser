/** @file */
#pragma once

#include <QtCore/QString>

/**
 * Returns the registrable/base domain for the given host.
 * E.g. google.com from www.google.com, amazon.co.uk from www.amazon.co.uk.
 * @param domain The domain (e.g. host from URL).
 * @return The base domain (last 2 or 3 labels depending on known two-part TLDs).
 */
[[nodiscard]] QString getBaseDomain(const QString &domain);
