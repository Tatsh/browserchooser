#include <QtTest/QTest>

#include "desktopentry.h"
#include "testdata_dir.h"

class DesktopEntryTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void parse_minimal_valid();
    void parse_nodisplay();
    void parse_invalid_returnsFalse();
};

void DesktopEntryTest::parse_minimal_valid() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    DesktopEntry entry(path + QStringLiteral("/minimal.desktop"));
    QVERIFY(entry.isValid());
    QVERIFY(!entry.exec().isEmpty());
    QVERIFY(!entry.name().isEmpty());
    QCOMPARE(entry.name(), QStringLiteral("Minimal Browser"));
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

QTEST_MAIN(DesktopEntryTest)
#include "desktopentrytest.moc"
