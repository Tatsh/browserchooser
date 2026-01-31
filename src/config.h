/** @file */
#pragma once

#include <expected>

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "backend.h"
#include "browseroption.h"

/** Error code for getRememberedBrowser. */
enum class GetRememberedBrowserError { EmptyDomain, InvalidPath, NotFound };

/** Manages saved browsers associated with domains. */
class SavedBrowsers {
public:
    /** Constructor. */
    SavedBrowsers();
    /**
     * Retrieves the remembered browser option for the given domain, if any.
     * @param domain The domain (e.g. host from URL) to look up.
     * @return The remembered BrowserOption on success, or an error.
     */
    [[nodiscard]] std::expected<BrowserOption, GetRememberedBrowserError>
    getRememberedBrowser(const QString &domain);
    /**
     * Remembers the browser option for the given domain.
     * @param domain The domain to associate with the browser.
     * @param option The BrowserOption to remember.
     */
    void remember(const QString &domain, const BrowserOption &option);
    /**
     * Forgets the remembered browser for the given domain.
     * @param domain The domain whose remembered browser should be forgotten.
     */
    void forget(const QString &domain);

private:
    QSettings settings_;
};

/** Application configuration management. */
class AppConfig {
public:
    /** Constructor. */
    AppConfig();
    /**
     * Retrieves the list of hidden browsers.
     * @return List of hidden browser names (e.g. @c .desktop base names).
     */
    [[nodiscard]] QStringList getHiddenBrowsers() const;
    /**
     * Retrieves the setting for including browsers with @c NoDisplay set.
     * @return @c IncludeNoDisplay::Yes or @c IncludeNoDisplay::No.
     */
    [[nodiscard]] IncludeNoDisplay includeNoDisplayBrowsers() const;
    /** Whether Guest profile options are hidden in the selector. Default true. */
    [[nodiscard]] bool hideGuestProfiles() const;
    /**
     * Sets whether to hide Guest profile options.
     * @param hide True to hide Guest profiles.
     */
    void setHideGuestProfiles(bool hide);
    /** Whether Guest profile options are shown (inverse of hideGuestProfiles). */
    [[nodiscard]] bool showGuestProfiles() const;
    /**
     * Sets whether to show Guest profile options.
     * @param show True to show Guest profiles.
     */
    void setShowGuestProfiles(bool show);
    /** Whether the "Other browsers" section is hidden (browsers without profiles). Default false. */
    [[nodiscard]] bool hideBrowsersWithoutProfiles() const;
    /**
     * Sets whether to hide the Other browsers section.
     * @param hide True to hide.
     */
    void setHideBrowsersWithoutProfiles(bool hide);
    /** Whether the "Do not ask again" checkbox was checked when the selector last closed. */
    [[nodiscard]] bool rememberChoiceChecked() const;
    /**
     * Sets whether the "Do not ask again" checkbox is checked.
     * @param checked True if checked.
     */
    void setRememberChoiceChecked(bool checked);
    /** Whether the "all subdomains" radio was last used (true) or "only this domain" (false). */
    [[nodiscard]] bool rememberDomainWildcard() const;
    /**
     * Sets which domain scope radio was last used.
     * @param wildcard True for all subdomains, false for only this domain.
     */
    void setRememberDomainWildcard(bool wildcard);

private:
    QSettings settings_;
};
