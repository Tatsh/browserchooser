#include <QtTest/QTest>

#include "browserinfoformat.h"

class BrowserInfoFormatTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void format_containsEscapedCommandAndPath();
    void format_emptyStrings();
    void format_htmlEscaping();
};

void BrowserInfoFormatTest::format_containsEscapedCommandAndPath() {
    const auto html = formatBrowserInfoHtml(QStringLiteral("cmd"), QStringLiteral("/path"));
    QVERIFY(html.contains(QStringLiteral("cmd")));
    QVERIFY(html.contains(QStringLiteral("/path")));
    QVERIFY(html.contains(QStringLiteral("Command line")));
    QVERIFY(html.contains(QStringLiteral("Desktop file")));
}

void BrowserInfoFormatTest::format_emptyStrings() {
    const auto html = formatBrowserInfoHtml(QString(), QString());
    QVERIFY(!html.isEmpty());
    QVERIFY(html.contains(QStringLiteral("Command line")));
}

void BrowserInfoFormatTest::format_htmlEscaping() {
    const auto html = formatBrowserInfoHtml(QStringLiteral("<script>"), QStringLiteral("&\"'"));
    QVERIFY(html.contains(QStringLiteral("&lt;script&gt;")));
    QVERIFY(html.contains(QStringLiteral("&amp;")));
}

QTEST_MAIN(BrowserInfoFormatTest)
#include "browserinfoformattest.moc"
