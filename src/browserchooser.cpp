#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include "backend.h"
#include "browserchooser.h"
#include "desktopentry.h"
#include "selectorwidget.h"

BrowserChooser::BrowserChooser(const QString &urlToOpen)
    : urlToOpen_(urlToOpen), appConfig_(), savedBrowsers_() {
    findBrowsers();
    parsedDomain_ = parseUrl(urlToOpen);
}

void BrowserChooser::findBrowsers() {
    availableBrowsers_ = getBrowsers(appConfig_.includeNoDisplayBrowsers());
    if (availableBrowsers_.isEmpty() &&
        appConfig_.includeNoDisplayBrowsers() == IncludeNoDisplay::No) {
        availableBrowsers_ = getBrowsers(IncludeNoDisplay::Yes);
    }
}

int BrowserChooser::exec() {
    if (parsedDomain_.has_value()) {
        auto remembered = savedBrowsers_.getRememberedBrowser(*parsedDomain_);
        if (remembered.has_value()) {
            openBrowser(remembered.value());
            return 0;
        }
    }
    removeHiddenBrowsers();
    if (availableBrowsers_.isEmpty()) {
        findBrowsers();
    }
    if (availableBrowsers_.size() == 1) {
        openBrowser(availableBrowsers_.first());
        return 0;
    }
    auto argc = 0;
    QApplication app(argc, nullptr);
    if (availableBrowsers_.isEmpty()) {
        QMessageBox::critical(
            nullptr, QObject::tr("Browser Chooser"), QObject::tr(R"(No web browsers found.

Please install a web browser with a .desktop file in a standard applications directory.)"));
        return 1;
    }
    // Show selector widget.
    SelectorWidget widget(this);
    widget.show();
    return app.exec();
}

void BrowserChooser::openBrowser(const BrowserOption &option) {
    QStringList urls;
    if (!urlToOpen_.isEmpty()) {
        urls.append(urlToOpen_);
    }
    launchBrowser(option, urls);
}

void BrowserChooser::remember(const BrowserOption &option, const QString &domainPattern) {
    auto pattern = domainPattern;
    if (pattern.isEmpty() && parsedDomain_.has_value()) {
        pattern = *parsedDomain_;
    }
    if (!pattern.isEmpty()) {
        savedBrowsers_.remember(pattern, option);
    }
}

bool BrowserChooser::showGuestProfiles() const {
    return appConfig_.showGuestProfiles();
}

void BrowserChooser::setShowGuestProfiles(bool show) {
    appConfig_.setShowGuestProfiles(show);
}

bool BrowserChooser::hideBrowsersWithoutProfiles() const {
    return appConfig_.hideBrowsersWithoutProfiles();
}

void BrowserChooser::setHideBrowsersWithoutProfiles(bool hide) {
    appConfig_.setHideBrowsersWithoutProfiles(hide);
}

bool BrowserChooser::rememberChoiceChecked() const {
    return appConfig_.rememberChoiceChecked();
}

void BrowserChooser::setRememberChoiceChecked(bool checked) {
    appConfig_.setRememberChoiceChecked(checked);
}

void BrowserChooser::removeHiddenBrowsers() {
    auto hiddenBrowsers = appConfig_.getHiddenBrowsers();
    availableBrowsers_.removeIf([&hiddenBrowsers](const BrowserOption &option) {
        return hiddenBrowsers.contains(QFileInfo(option.entry().filename()).completeBaseName());
    });
}
