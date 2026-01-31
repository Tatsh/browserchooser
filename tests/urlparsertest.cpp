#include <QtTest/QTest>

#include "urlparser.h"

class UrlParserTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void parseUrl_empty_returnsError();
    void parseUrl_https_returnsHost();
    void parseUrl_file_returnsFileName();
    void parseUrl_barePath_returnsFileName();
};

void UrlParserTest::parseUrl_empty_returnsError() {
    const auto result = parseUrl(QString());
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), ParseUrlError::EmptyUrl);
}

void UrlParserTest::parseUrl_https_returnsHost() {
    const auto result = parseUrl(QStringLiteral("https://a.github.io/path"));
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), QStringLiteral("a.github.io"));
}

void UrlParserTest::parseUrl_file_returnsFileName() {
    const auto result = parseUrl(QStringLiteral("file:///tmp/foo.pdf"));
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), QStringLiteral("foo.pdf"));
}

void UrlParserTest::parseUrl_barePath_returnsFileName() {
    const auto result = parseUrl(QStringLiteral("/tmp/bar.pdf"));
    QVERIFY(result.has_value());
    QCOMPARE(result.value(), QStringLiteral("bar.pdf"));
}

QTEST_MAIN(UrlParserTest)
#include "urlparsertest.moc"
