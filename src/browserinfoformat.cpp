#include "browserinfoformat.h"

#include <QtCore/QString>

static const auto kHtmlNowrap = QStringLiteral("<span style='white-space: nowrap;'>");
static const auto kHtmlEndSpan = QStringLiteral("</span>");
static const auto kHtmlFormat =
    QStringLiteral("<span style='font-weight: normal; font-size: 11px;'>"
                   "%1<h3>Command line</h3><code>%2</code>%3<br>"
                   "%1<h3>Desktop file</h3><code>%4</code>%3</span>");

QString formatBrowserInfoHtml(const QString &commandLine, const QString &desktopPath) {
    return kHtmlFormat.arg(
        kHtmlNowrap, commandLine.toHtmlEscaped(), kHtmlEndSpan, desktopPath.toHtmlEscaped());
}
