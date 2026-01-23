#include <QtCore/QDir>
#include <QtCore/QRegularExpression>
#include <QtCore/QStandardPaths>

#include "config.h"

QString getConfigFilePath() {
    auto configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + QStringLiteral("/browserselectorrc");
}

namespace {

    auto expandTilde(const QString &path) -> QString {
        if (path.startsWith(QStringLiteral("~/"))) {
            return QDir::homePath() + path.mid(1);
        }
        if (path == QStringLiteral("~")) {
            return QDir::homePath();
        }
        return path;
    }

    auto matchesWildcardPattern(const QString &pattern, const QString &domain) {
        // Special handling for *. prefix: match any subdomain including no subdomain
        // e.g., *.google.com matches www.google.com, mail.google.com, and google.com
        if (pattern.startsWith(QStringLiteral("*."))) {
            auto baseDomain = pattern.mid(2); // Remove "*."

            // Check if domain equals the base domain (no subdomain)
            if (domain.compare(baseDomain, Qt::CaseInsensitive) == 0) {
                return true;
            }

            // Check if domain ends with .baseDomain (has subdomain)
            if (domain.endsWith(QLatin1Char('.') + baseDomain, Qt::CaseInsensitive)) {
                return true;
            }

            return false;
        }

        // Standard wildcard matching for other patterns
        auto regexPattern = QRegularExpression::wildcardToRegularExpression(
            pattern, QRegularExpression::UnanchoredWildcardConversion);
        QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);
        return regex.match(domain).hasMatch();
    }

} // anonymous namespace

// SavedBrowsers implementation

SavedBrowsers::SavedBrowsers() : settings_(getConfigFilePath(), QSettings::IniFormat) {
}

std::expected<DesktopEntry, QString> SavedBrowsers::getRememberedBrowser(const QString &domain) {
    if (domain.isEmpty()) {
        return std::unexpected(QStringLiteral("Empty domain"));
    }

    settings_.beginGroup(QStringLiteral("RememberedBrowsers"));

    // First try exact match
    auto browserPath = settings_.value(domain).toString();
    if (!browserPath.isEmpty()) {
        settings_.endGroup();
        return readDesktopEntry(expandTilde(browserPath));
    }

    // Try wildcard patterns
    auto keys = settings_.childKeys();
    for (const auto &pattern : keys) {
        // Skip if no wildcards present
        if (!pattern.contains(QLatin1Char('*')) && !pattern.contains(QLatin1Char('?'))) {
            continue;
        }

        if (matchesWildcardPattern(pattern, domain)) {
            browserPath = settings_.value(pattern).toString();
            settings_.endGroup();
            return readDesktopEntry(expandTilde(browserPath));
        }
    }

    settings_.endGroup();
    return std::unexpected(QStringLiteral("No remembered browser for domain"));
}

void SavedBrowsers::remember(const QString &domain, const DesktopEntry &entry) {
    settings_.beginGroup(QStringLiteral("RememberedBrowsers"));
    settings_.setValue(domain, entry.filename());
    settings_.endGroup();
    settings_.sync();
}

void SavedBrowsers::forget(const QString &domain) {
    settings_.beginGroup(QStringLiteral("RememberedBrowsers"));
    settings_.remove(domain);
    settings_.endGroup();
    settings_.sync();
}

// AppConfig implementation

AppConfig::AppConfig() : settings_(getConfigFilePath(), QSettings::IniFormat) {
}

QStringList AppConfig::getHiddenBrowsers() const {
    return settings_.value(QStringLiteral("General/hidden_browsers")).toStringList();
}

IncludeNoDisplay AppConfig::includeNoDisplayBrowsers() const {
    return settings_.value(QStringLiteral("General/include_no_display_browsers"), false).toBool() ?
               IncludeNoDisplay::Yes :
               IncludeNoDisplay::No;
}
