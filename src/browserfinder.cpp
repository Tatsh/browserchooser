#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QStandardPaths>

#include "browserfinder.h"

QList<DesktopEntry> getBrowsers(IncludeNoDisplay includeNoDisplay) {
    // QStandardPaths returns directories in order of preference,
    // with user directories (~/.local/share/applications) first
    auto appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    QList<DesktopEntry> browsers;
    QSet<QString> seenNames; // Dedupe by display name
    for (const auto &appDir : appDirs) {
        QDir dir(appDir);
        if (!dir.exists()) {
            continue;
        }
        auto desktopFiles = dir.entryList({QStringLiteral("*.desktop")}, QDir::Files);
        for (const auto &desktopFile : desktopFiles) {
            auto fullPath = dir.absoluteFilePath(desktopFile);
            auto entryOpt = readDesktopEntry(fullPath);
            if (!entryOpt.has_value()) {
                continue;
            }
            auto entry = entryOpt.value();
            // Skip ourselves
            if (entry.startupWMClass() == QStringLiteral("browserselector")) {
                continue;
            }
            // Skip NoDisplay entries unless explicitly included
            if (includeNoDisplay == IncludeNoDisplay::No && entry.noDisplay()) {
                continue;
            }
            // Check if it's a web browser
            auto categories = entry.categories();
            auto mimeTypes = entry.mimeTypes();
            auto isWebBrowser = categories.contains(QStringLiteral("WebBrowser"));
            auto handlesHttp = mimeTypes.contains(QStringLiteral("x-scheme-handler/http")) ||
                               mimeTypes.contains(QStringLiteral("x-scheme-handler/https"));
            if (isWebBrowser && handlesHttp) {
                // Dedupe by display name - first occurrence wins (user dirs preferred)
                if (!seenNames.contains(entry.name())) {
                    seenNames.insert(entry.name());
                    browsers.append(entry);
                }
            }
        }
    }
    // Sort by browser name (case-insensitive)
    std::ranges::sort(browsers, [](const DesktopEntry &a, const DesktopEntry &b) {
        return a.name().compare(b.name(), Qt::CaseInsensitive) < 0;
    });
    return browsers;
}
