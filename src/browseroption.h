/** @file */
#pragma once

#include <QtCore/QString>

class DesktopEntry;

/**
 * A selectable browser option: a desktop entry plus an optional Chrome/Chromium
 * profile. Default profile is represented by an empty profile name.
 */
class BrowserOption {
public:
    /**
     * Construct from @c .desktop file path, profile ID (for launch), and optional
     * profile display name (for UI). If display name is empty, profile ID is used.
     * @param singleProfile When true, displayName() returns only the browser name.
     * @param fromProfileDiscovery When true, this option came from profile discovery.
     */
    BrowserOption(const QString &desktopPath,
                  const QString &profileId = QString(),
                  const QString &profileDisplayName = QString(),
                  bool singleProfile = false,
                  bool fromProfileDiscovery = false);
    /** Construct from a desktop entry, profile ID, and optional display name. */
    BrowserOption(const DesktopEntry &entry,
                  const QString &profileId = QString(),
                  const QString &profileDisplayName = QString(),
                  bool singleProfile = false,
                  bool fromProfileDiscovery = false);

    /** Path to the @c .desktop file. */
    [[nodiscard]] QString desktopPath() const {
        return desktopPath_;
    }
    /** Profile ID for launch (e.g. @c --profile= or @c -P); empty for Default. */
    [[nodiscard]] QString profileName() const {
        return profileName_;
    }
    /** Display name, e.g. "Google Chrome Beta (Default)" or "Google Chrome Beta (Work)". */
    [[nodiscard]] QString displayName() const;
    /** Profile label for use under a browser section header (e.g. "Default", "Work"). */
    [[nodiscard]] QString profileLabel() const;
    /** Underlying desktop entry. */
    [[nodiscard]] DesktopEntry entry() const;
    /** Whether this option is valid (@c .desktop file exists and parses). */
    [[nodiscard]] bool isValid() const;
    /** Whether this option came from profile discovery (Chrome/Firefox profiles). */
    [[nodiscard]] bool fromProfileDiscovery() const {
        return fromProfileDiscovery_;
    }

private:
    QString desktopPath_;
    QString profileName_;
    QString profileDisplayName_;
    bool singleProfile_ = false;
    bool fromProfileDiscovery_ = false;
};

/** Returns true if both options refer to the same browser and profile. */
bool operator==(const BrowserOption &a, const BrowserOption &b);
