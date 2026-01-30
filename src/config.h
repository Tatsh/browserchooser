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
    /** Whether Guest profile options are shown in the selector. */
    [[nodiscard]] bool showGuestProfiles() const;
    /** Sets whether Guest profile options are shown. @param show True to show Guest profiles. */
    void setShowGuestProfiles(bool show);
    /** Whether the "Do not ask again" checkbox was checked when the selector last closed. */
    [[nodiscard]] bool rememberChoiceChecked() const;
    /** Sets whether the "Do not ask again" checkbox is checked. @param checked True if checked. */
    void setRememberChoiceChecked(bool checked);

private:
    QSettings settings_;
};
