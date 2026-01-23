#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include "browserfinder.h"
#include "browserlauncher.h"
#include "browserselector.h"
#include "selectorwidget.h"

BrowserSelector::BrowserSelector(const QString &urlToOpen)
    : urlToOpen_(urlToOpen), appConfig_(), savedBrowsers_() {
    findBrowsers();
    parsedDomain_ = parseUrl(urlToOpen);
}

void BrowserSelector::findBrowsers() {
    // Try to find browsers with current settings
    availableBrowsers_ = getBrowsers(appConfig_.includeNoDisplayBrowsers());
    // If no browsers found, try including NoDisplay browsers
    if (availableBrowsers_.isEmpty() &&
        appConfig_.includeNoDisplayBrowsers() == IncludeNoDisplay::No) {
        availableBrowsers_ = getBrowsers(IncludeNoDisplay::Yes);
    }
}

int BrowserSelector::exec() {
    // Check for remembered browser
    if (parsedDomain_.has_value()) {
        auto remembered = savedBrowsers_.getRememberedBrowser(*parsedDomain_);
        if (remembered.has_value()) {
            openBrowser(remembered.value());
            return 0;
        }
    }
    removeHiddenBrowsers();
    // If no browsers after filtering, try again without filtering
    if (availableBrowsers_.isEmpty()) {
        findBrowsers();
    }
    auto argc = 0;
    QApplication app(argc, nullptr);
    // Still no browsers - show error and exit
    if (availableBrowsers_.isEmpty()) {
        QMessageBox::critical(nullptr,
                              QObject::tr("Browser Selector"),
                              QObject::tr("No web browsers found.\n\n"
                                          "Please install a web browser with a .desktop file "
                                          "in a standard applications directory."));
        return 1;
    }
    // Show selector widget
    SelectorWidget widget(this);
    widget.show();
    return app.exec();
}

void BrowserSelector::openBrowser(const DesktopEntry &entry) {
    QStringList urls;
    if (!urlToOpen_.isEmpty()) {
        urls.append(urlToOpen_);
    }
    launchBrowser(entry, urls);
}

void BrowserSelector::remember(const DesktopEntry &entry, const QString &domainPattern) {
    auto pattern = domainPattern;
    if (pattern.isEmpty() && parsedDomain_.has_value()) {
        pattern = *parsedDomain_;
    }
    if (!pattern.isEmpty()) {
        savedBrowsers_.remember(pattern, entry);
    }
}

void BrowserSelector::removeHiddenBrowsers() {
    auto hiddenBrowsers = appConfig_.getHiddenBrowsers();
    availableBrowsers_.removeIf([&hiddenBrowsers](const DesktopEntry &browser) {
        return hiddenBrowsers.contains(QFileInfo(browser.filename()).completeBaseName());
    });
}
