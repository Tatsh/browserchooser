#include <QtCore/QDir>
#include <QtCore/QRegularExpression>
#include <QtCore/QStandardPaths>

#include "backend.h"
#include "browseroption.h"
#include "config.h"

namespace {

static const auto kTildeSlash = QStringLiteral("~/");
static const auto kTilde = QStringLiteral("~");
static const auto kWildcardPrefix = QStringLiteral("*.");
static const auto kRememberedBrowsers = QStringLiteral("RememberedBrowsers");
static const auto kKeyHiddenBrowsers = QStringLiteral("hidden_browsers");
static const auto kKeyIncludeNoDisplayBrowsers = QStringLiteral("include_no_display_browsers");
static const auto kKeyHideGuestProfiles = QStringLiteral("hide_guest_profiles");
static const auto kKeyHideBrowsersWithoutProfiles =
    QStringLiteral("hide_browsers_without_profiles");
static const auto kKeyRememberChoiceChecked = QStringLiteral("remember_choice_checked");
static const auto kKeyRememberDomainWildcard = QStringLiteral("remember_domain_wildcard");

auto expandTilde(const QString &path) -> QString {
    if (path.startsWith(kTildeSlash)) {
        return QDir::homePath() + path.mid(1);
    }
    if (path == kTilde) {
        return QDir::homePath();
    }
    return path;
}

auto matchesWildcardPattern(const QString &pattern, const QString &domain) {
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

} // anonymous namespace

// SavedBrowsers implementation.

SavedBrowsers::SavedBrowsers() : settings_(getConfigFilePath(), QSettings::IniFormat) {
}

std::expected<BrowserOption, GetRememberedBrowserError>
SavedBrowsers::getRememberedBrowser(const QString &domain) {
    if (domain.isEmpty()) {
        return std::unexpected(GetRememberedBrowserError::EmptyDomain);
    }

    settings_.beginGroup(kRememberedBrowsers);

    auto value = settings_.value(domain).toString();
    if (!value.isEmpty()) {
        settings_.endGroup();
        auto parts = value.split(QLatin1Char('|'));
        auto desktopPath = expandTilde(parts.first());
        auto profileId = parts.size() > 1 ? parts.at(1) : QString();
        auto displayName = getChromeProfileDisplayName(desktopPath, profileId);
        BrowserOption option(desktopPath, profileId, displayName);
        if (option.isValid()) {
            return option;
        }
        return std::unexpected(GetRememberedBrowserError::InvalidPath);
    }

    auto keys = settings_.childKeys();
    for (const auto &pattern : keys) {
        if (pattern.isEmpty() ||
            (!pattern.contains(QLatin1Char('*')) && !pattern.contains(QLatin1Char('?')))) {
            continue;
        }
        if (matchesWildcardPattern(pattern, domain)) {
            value = settings_.value(pattern).toString();
            settings_.endGroup();
            auto parts = value.split(QLatin1Char('|'));
            auto desktopPath = expandTilde(parts.first());
            auto profileId = parts.size() > 1 ? parts.at(1) : QString();
            auto displayName = getChromeProfileDisplayName(desktopPath, profileId);
            BrowserOption option(desktopPath, profileId, displayName);
            if (option.isValid()) {
                return option;
            }
            return std::unexpected(GetRememberedBrowserError::InvalidPath);
        }
    }

    settings_.endGroup();
    return std::unexpected(GetRememberedBrowserError::NotFound);
}

void SavedBrowsers::remember(const QString &domain, const BrowserOption &option) {
    if (domain.isEmpty()) {
        return;
    }
    settings_.beginGroup(kRememberedBrowsers);
    auto value = option.desktopPath();
    if (!option.profileName().isEmpty()) {
        value += QLatin1Char('|') + option.profileName();
    }
    settings_.setValue(domain, value);
    settings_.endGroup();
    settings_.sync();
}

void SavedBrowsers::forget(const QString &domain) {
    if (domain.isEmpty()) {
        return;
    }
    settings_.beginGroup(kRememberedBrowsers);
    settings_.remove(domain);
    settings_.endGroup();
    settings_.sync();
}

// AppConfig implementation.

AppConfig::AppConfig() : settings_(getConfigFilePath(), QSettings::IniFormat) {
}

QStringList AppConfig::getHiddenBrowsers() const {
    return settings_.value(kKeyHiddenBrowsers).toStringList();
}

IncludeNoDisplay AppConfig::includeNoDisplayBrowsers() const {
    return settings_.value(kKeyIncludeNoDisplayBrowsers, false).toBool() ? IncludeNoDisplay::Yes :
                                                                           IncludeNoDisplay::No;
}

bool AppConfig::hideGuestProfiles() const {
    return settings_.value(kKeyHideGuestProfiles, true).toBool();
}

void AppConfig::setHideGuestProfiles(bool hide) {
    settings_.setValue(kKeyHideGuestProfiles, hide);
    settings_.sync();
}

bool AppConfig::showGuestProfiles() const {
    return !hideGuestProfiles();
}

void AppConfig::setShowGuestProfiles(bool show) {
    setHideGuestProfiles(!show);
}

bool AppConfig::hideBrowsersWithoutProfiles() const {
    return settings_.value(kKeyHideBrowsersWithoutProfiles, false).toBool();
}

void AppConfig::setHideBrowsersWithoutProfiles(bool hide) {
    settings_.setValue(kKeyHideBrowsersWithoutProfiles, hide);
    settings_.sync();
}

bool AppConfig::rememberChoiceChecked() const {
    return settings_.value(kKeyRememberChoiceChecked, true).toBool();
}

void AppConfig::setRememberChoiceChecked(bool checked) {
    settings_.setValue(kKeyRememberChoiceChecked, checked);
    settings_.sync();
}

bool AppConfig::rememberDomainWildcard() const {
    return settings_.value(kKeyRememberDomainWildcard, true).toBool();
}

void AppConfig::setRememberDomainWildcard(bool wildcard) {
    settings_.setValue(kKeyRememberDomainWildcard, wildcard);
    settings_.sync();
}
