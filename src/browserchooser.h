/** @file */
#pragma once

#include <expected>

#include <QtCore/QList>
#include <QtCore/QString>

#include "browseroption.h"
#include "config.h"
#include "urlparser.h"

/** Manages browser selection based on a URL. */
class BrowserChooser {
public:
    /**
     * Constructor.
     * @param urlToOpen The URL to be opened in the selected browser.
     */
    explicit BrowserChooser(const QString &urlToOpen = QString());
    /**
      * Executes the browser chooser dialog.
      * @return Exit code (0 on success).
      */
    [[nodiscard]] int exec();
    /**
     * Open the specified browser option.
     * @param option The browser and optional profile to open.
     */
    void openBrowser(const BrowserOption &option);
    /**
     * Remember the browser choice for the given domain pattern.
     * @param option The browser and optional profile to remember.
     * @param domainPattern The domain pattern to associate with the browser.
     */
    void remember(const BrowserOption &option, const QString &domainPattern = QString());

    /** Get the URL to be opened. */
    [[nodiscard]] const QString &urlToOpen() const {
        return urlToOpen_;
    }
    /** Get the parsed domain from the URL. */
    [[nodiscard]] const std::expected<QString, ParseUrlError> &parsedDomain() const {
        return parsedDomain_;
    }
    /** Get the list of available browser options. */
    [[nodiscard]] const QList<BrowserOption> &availableBrowsers() const {
        return availableBrowsers_;
    }
    /** Whether Guest profile options are hidden. Default true. */
    [[nodiscard]] bool hideGuestProfiles() const;
    /** Sets whether to hide Guest profile options. */
    void setHideGuestProfiles(bool hide);
    /** Whether Guest profile options are shown (inverse of hideGuestProfiles). */
    [[nodiscard]] bool showGuestProfiles() const;
    /** Sets whether to show Guest profile options. */
    void setShowGuestProfiles(bool show);
    /** Whether the Other browsers section is hidden. */
    [[nodiscard]] bool hideBrowsersWithoutProfiles() const;
    /** Sets whether to hide the Other browsers section. */
    void setHideBrowsersWithoutProfiles(bool hide);
    /** Whether the "Do not ask again" checkbox was checked when the selector last closed. */
    [[nodiscard]] bool rememberChoiceChecked() const;
    /** Sets whether the "Do not ask again" checkbox is checked. */
    void setRememberChoiceChecked(bool checked);
    /** Whether the "all subdomains" radio was last used. */
    [[nodiscard]] bool rememberDomainWildcard() const;
    /** Sets which domain scope radio was last used. */
    void setRememberDomainWildcard(bool wildcard);

private:
    void findBrowsers();
    void removeHiddenBrowsers();

    QString urlToOpen_;
    std::expected<QString, ParseUrlError> parsedDomain_;
    AppConfig appConfig_;
    SavedBrowsers savedBrowsers_;
    QList<BrowserOption> availableBrowsers_;
};
