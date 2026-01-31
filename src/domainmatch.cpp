#include "domainmatch.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QString>

static const auto kWildcardPrefix = QStringLiteral("*.");

bool matchesWildcardPattern(const QString &pattern, const QString &domain) {
    // Special handling for *. prefix: match any subdomain including no subdomain.
    // e.g., *.google.com matches www.google.com, mail.google.com, and google.com
    if (pattern.startsWith(kWildcardPrefix)) {
        auto baseDomain = pattern.mid(2); // Remove "*." prefix.

        // Check if domain equals the base domain (no subdomain).
        if (domain.compare(baseDomain, Qt::CaseInsensitive) == 0) {
            return true;
        }

        // Check if domain ends with .baseDomain (has subdomain).
        if (domain.endsWith(QLatin1Char('.') + baseDomain, Qt::CaseInsensitive)) {
            return true;
        }

        return false;
    }

    // Standard wildcard matching for other patterns.
    auto regexPattern = QRegularExpression::wildcardToRegularExpression(
        pattern, QRegularExpression::UnanchoredWildcardConversion);
    QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);
    return regex.match(domain).hasMatch();
}
