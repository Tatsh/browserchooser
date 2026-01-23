#include <QtCore/QFileInfo>
#include <QtCore/QUrl>

#include "urlparser.h"

std::expected<QString, QString> parseUrl(const QString &url) {
    if (url.isEmpty()) {
        return std::unexpected(QStringLiteral("Empty URL"));
    }
    QUrl parsedUrl(url);
    auto scheme = parsedUrl.scheme();
    if (scheme == QStringLiteral("file") || scheme.isEmpty()) {
        // For file URLs, use the filename as domain
        auto path = parsedUrl.path();
        if (path.isEmpty()) {
            path = url; // Handle bare file paths
        }
        return QFileInfo(path).fileName();
    }
    // For web URLs, use the host as domain
    return parsedUrl.host();
}
