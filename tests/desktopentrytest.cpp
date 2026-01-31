#include <QtCore/QLocale>
#include <QtTest/QTest>

#include "desktopentry.h"
#include "testdatadir.h"

class DesktopEntryTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void parse_minimal_valid();
    void parse_nodisplay();
    void parse_invalid_returnsFalse();
    void getLocalizedValue_fullLocale_returnsValue();
    void getLocalizedValue_langOnly_returnsValue();
    void parse_nodisplay_altCase_findsKey();
    void comment_returnsGetLocalizedValue();
    void parse_categories_getListValue();
    void executableName_empty_returnsEmpty();
    void executableName_withQuotes_handlesQuotedArg();
    void executableName_quotedFirstToken_returnsPath();
    void executableName_percentOnly_returnsEmpty();
    void executableName_returnsFirstToken();
    void executableName_simpleAppendsChars();
    void readDesktopEntry_nonexistent_returnsParseFailed();
};

void DesktopEntryTest::parse_minimal_valid() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/minimal.desktop"));
    QVERIFY(entry.isValid());
    QVERIFY(!entry.exec().isEmpty());
    QVERIFY(!entry.name().isEmpty());
    QCOMPARE(entry.name(), QStringLiteral("Minimal Browser"));
    QCOMPARE(entry.executableName(), QStringLiteral("/usr/bin/minimal-browser"));
}

void DesktopEntryTest::parse_nodisplay() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/nodisplay.desktop"));
    QVERIFY(entry.isValid());
    QVERIFY(entry.noDisplay());
}

void DesktopEntryTest::parse_invalid_returnsFalse() {
    DesktopEntry entry;
    QVERIFY(!entry.parse(QStringLiteral("/nonexistent/file.desktop")));
    QVERIFY(!entry.isValid());
}

void DesktopEntryTest::getLocalizedValue_fullLocale_returnsValue() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    QLocale::setDefault(QLocale(QStringLiteral("en_US")));
    DesktopEntry entry(path + QStringLiteral("/localized.desktop"));
    QVERIFY(entry.isValid());
    QCOMPARE(entry.name(), QStringLiteral("US English"));
}

void DesktopEntryTest::getLocalizedValue_langOnly_returnsValue() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    QLocale::setDefault(QLocale(QStringLiteral("en_GB")));
    DesktopEntry entry(path + QStringLiteral("/localized.desktop"));
    QVERIFY(entry.isValid());
    QCOMPARE(entry.name(), QStringLiteral("English"));
}

void DesktopEntryTest::parse_nodisplay_altCase_findsKey() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/nodisplay_altcase.desktop"));
    QVERIFY(entry.isValid());
    QVERIFY(entry.noDisplay());
}

void DesktopEntryTest::comment_returnsGetLocalizedValue() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/minimal.desktop"));
    QVERIFY(entry.isValid());
    QVERIFY(entry.comment().isEmpty());
}

void DesktopEntryTest::parse_categories_getListValue() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/categories.desktop"));
    QVERIFY(entry.isValid());
    QCOMPARE(entry.categories().size(), 3);
}

void DesktopEntryTest::executableName_empty_returnsEmpty() {
    DesktopEntry entry;
    QVERIFY(entry.executableName().isEmpty());
}

void DesktopEntryTest::executableName_withQuotes_handlesQuotedArg() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/exec_quoted.desktop"));
    QVERIFY(entry.isValid());
    QCOMPARE(entry.executableName(), QStringLiteral("/usr/bin/browser"));
}

void DesktopEntryTest::executableName_quotedFirstToken_returnsPath() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/exec_quoted_first.desktop"));
    QVERIFY(entry.isValid());
    QCOMPARE(entry.executableName(), QStringLiteral("/usr/bin/browser"));
}

void DesktopEntryTest::executableName_percentOnly_returnsEmpty() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/exec_percent_only.desktop"));
    QVERIFY(entry.isValid());
    QVERIFY(entry.executableName().isEmpty());
}

void DesktopEntryTest::executableName_returnsFirstToken() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/minimal.desktop"));
    QVERIFY(entry.isValid());
    QCOMPARE(entry.executableName(), QStringLiteral("/usr/bin/minimal-browser"));
}

void DesktopEntryTest::executableName_simpleAppendsChars() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/exec_simple.desktop"));
    QVERIFY(entry.isValid());
    QCOMPARE(entry.executableName(), QStringLiteral("foo"));
}

void DesktopEntryTest::readDesktopEntry_nonexistent_returnsParseFailed() {
    auto result = readDesktopEntry(QStringLiteral("/nonexistent/file.desktop"));
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), DesktopEntryError::ParseFailed);
}

QTEST_MAIN(DesktopEntryTest)
#include "desktopentrytest.moc"
