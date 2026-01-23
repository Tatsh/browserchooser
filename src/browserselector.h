#pragma once

#include <expected>

#include <QtCore/QList>
#include <QtCore/QString>

#include "config.h"
#include "desktopentry.h"
#include "urlparser.h"

/** Manages browser selection based on a URL. */
class BrowserSelector {
public:
    /**
     * Constructor.
     * @param urlToOpen The URL to be opened in the selected browser.
     */
    explicit BrowserSelector(const QString &urlToOpen = QString());
    /** Execute the browser selector dialog. */
    [[nodiscard]] int exec();
    /**
     * Open the specified browser.
     * @param entry The DesktopEntry of the browser to open.
     */
    void openBrowser(const DesktopEntry &entry);
    /**
     * Remember the browser choice for the given domain pattern.
     * @param entry The DesktopEntry of the browser to remember.
     * @param domainPattern The domain pattern to associate with the browser.
     */
    void remember(const DesktopEntry &entry, const QString &domainPattern = QString());

    /** Get the URL to be opened. */
    [[nodiscard]] const QString &urlToOpen() const {
        return urlToOpen_;
    }
    /** Get the parsed domain from the URL. */
    [[nodiscard]] const std::expected<QString, QString> &parsedDomain() const {
        return parsedDomain_;
    }
    /** Get the list of available browsers. */
    [[nodiscard]] const QList<DesktopEntry> &availableBrowsers() const {
        return availableBrowsers_;
    }

private:
    void findBrowsers();
    void removeHiddenBrowsers();

    QString urlToOpen_;
    std::expected<QString, QString> parsedDomain_;
    AppConfig appConfig_;
    SavedBrowsers savedBrowsers_;
    QList<DesktopEntry> availableBrowsers_;
};
