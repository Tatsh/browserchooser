#include "browseroption.h"
#include "desktopentry.h"

BrowserOption::BrowserOption(const QString &desktopPath,
                             const QString &profileId,
                             const QString &profileDisplayName,
                             bool singleProfile,
                             bool fromProfileDiscovery)
    : desktopPath_(desktopPath), profileName_(profileId), profileDisplayName_(profileDisplayName),
      singleProfile_(singleProfile), fromProfileDiscovery_(fromProfileDiscovery) {
}

BrowserOption::BrowserOption(const DesktopEntry &entry,
                             const QString &profileId,
                             const QString &profileDisplayName,
                             bool singleProfile,
                             bool fromProfileDiscovery)
    : desktopPath_(entry.filename()), profileName_(profileId),
      profileDisplayName_(profileDisplayName), singleProfile_(singleProfile),
      fromProfileDiscovery_(fromProfileDiscovery) {
}

QString BrowserOption::displayName() const {
    auto e = entry();
    if (!e.isValid()) {
        return {};
    }
    if (singleProfile_) {
        return e.name();
    }
    if (profileName_.isEmpty()) {
        return e.name() + QLatin1String(" (Default)");
    }
    auto label = profileDisplayName_.isEmpty() ? profileName_ : profileDisplayName_;
    return e.name() + QLatin1String(" (") + label + QLatin1Char(')');
}

QString BrowserOption::profileLabel() const {
    if (profileName_.isEmpty()) {
        return QStringLiteral("Default");
    }
    return profileDisplayName_.isEmpty() ? profileName_ : profileDisplayName_;
}

DesktopEntry BrowserOption::entry() const {
    auto result = readDesktopEntry(desktopPath_);
    if (result.has_value()) {
        return result.value();
    }
    return DesktopEntry();
}

bool BrowserOption::isValid() const {
    return readDesktopEntry(desktopPath_).has_value();
}

bool operator==(const BrowserOption &a, const BrowserOption &b) {
    return a.desktopPath() == b.desktopPath() && a.profileName() == b.profileName();
}
