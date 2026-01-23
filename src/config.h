#pragma once

#include <expected>

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "browserfinder.h"
#include "desktopentry.h"

/** Returns the path to the configuration file. */
[[nodiscard]] QString getConfigFilePath();

/** Manages saved browsers associated with domains. */
class SavedBrowsers {
public:
    /** Constructor. */
    SavedBrowsers();
    /**
     * Retrieves the remembered browser for the given domain, if any.
     * @param domain The domain to look up.
     */
    [[nodiscard]] std::expected<DesktopEntry, QString> getRememberedBrowser(const QString &domain);
    /**
     * Remembers the browser for the given domain.
     * @param domain The domain to associate with the browser.
     * @param entry The DesktopEntry of the browser to remember.
     */
    void remember(const QString &domain, const DesktopEntry &entry);
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
    /** Retrieves the list of hidden browsers. */
    [[nodiscard]] QStringList getHiddenBrowsers() const;
    /** Retrieves the setting for including browsers with NoDisplay set. */
    [[nodiscard]] IncludeNoDisplay includeNoDisplayBrowsers() const;

private:
    QSettings settings_;
};
