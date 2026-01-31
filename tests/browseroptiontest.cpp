#include <QtCore/QCoreApplication>
#include <QtTest/QTest>

#include "browseroption.h"
#include "desktopentry.h"
#include "stringconstants.h"
#include "testdatadir.h"

class BrowserOptionTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void constructor_fromDesktopPath();
    void constructor_fromDesktopEntry();
    void displayName_invalidEntry_returnsEmpty();
    void displayName_singleProfile_returnsBrowserName();
    void displayName_defaultProfile_returnsBrowserNameWithDefault();
    void displayName_withProfileDisplayName_returnsBrowserNameWithLabel();
    void displayName_withProfileNameOnly_returnsBrowserNameWithProfileId();
    void profileLabel_emptyProfileName_returnsDefault();
    void profileLabel_nonEmptyProfileName_emptyDisplayName_returnsProfileName();
    void profileLabel_nonEmptyProfileName_withDisplayName_returnsDisplayName();
    void entry_invalidPath_returnsInvalidDesktopEntry();
    void operatorEquals_samePathAndProfile_returnsTrue();
    void operatorEquals_differentPath_returnsFalse();
    void operatorEquals_differentProfile_returnsFalse();
};

void BrowserOptionTest::constructor_fromDesktopPath() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption option(path);
    QCOMPARE(option.desktopPath(), path);
    QVERIFY(option.profileName().isEmpty());
    QVERIFY(option.isValid());
}

void BrowserOptionTest::constructor_fromDesktopEntry() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    DesktopEntry entry(path);
    BrowserOption option(entry);
    QCOMPARE(option.desktopPath(), path);
    QVERIFY(option.isValid());
}

void BrowserOptionTest::displayName_invalidEntry_returnsEmpty() {
    BrowserOption option(QStringLiteral("/nonexistent/file.desktop"));
    QVERIFY(option.displayName().isEmpty());
}

void BrowserOptionTest::displayName_singleProfile_returnsBrowserName() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption option(path, QString(), QString(), true);
    QCOMPARE(option.displayName(), QStringLiteral("Minimal Browser"));
}

void BrowserOptionTest::displayName_defaultProfile_returnsBrowserNameWithDefault() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption option(path, QString(), QString(), false);
    QCOMPARE(option.displayName(), QStringLiteral("Minimal Browser (Default)"));
}

void BrowserOptionTest::displayName_withProfileDisplayName_returnsBrowserNameWithLabel() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption option(path, QStringLiteral("Profile 1"), QStringLiteral("Work"), false);
    QCOMPARE(option.displayName(), QStringLiteral("Minimal Browser (Work)"));
}

void BrowserOptionTest::displayName_withProfileNameOnly_returnsBrowserNameWithProfileId() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption option(path, QStringLiteral("Profile 1"), QString(), false);
    QCOMPARE(option.displayName(), QStringLiteral("Minimal Browser (Profile 1)"));
}

void BrowserOptionTest::profileLabel_emptyProfileName_returnsDefault() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption option(path, QString(), QString(), false);
    QCOMPARE(option.profileLabel(), kDefault);
}

void BrowserOptionTest::profileLabel_nonEmptyProfileName_emptyDisplayName_returnsProfileName() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption option(path, QStringLiteral("Profile 1"), QString(), false);
    QCOMPARE(option.profileLabel(), QStringLiteral("Profile 1"));
}

void BrowserOptionTest::profileLabel_nonEmptyProfileName_withDisplayName_returnsDisplayName() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption option(path, QStringLiteral("Profile 1"), QStringLiteral("Work"), false);
    QCOMPARE(option.profileLabel(), QStringLiteral("Work"));
}

void BrowserOptionTest::entry_invalidPath_returnsInvalidDesktopEntry() {
    BrowserOption option(QStringLiteral("/nonexistent/file.desktop"));
    const auto e = option.entry();
    QVERIFY(!e.isValid());
}

void BrowserOptionTest::operatorEquals_samePathAndProfile_returnsTrue() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption a(path, QStringLiteral("Profile 1"), QStringLiteral("Work"), false);
    BrowserOption b(path, QStringLiteral("Profile 1"), QStringLiteral("Other"), false);
    QVERIFY(a == b);
}

void BrowserOptionTest::operatorEquals_differentPath_returnsFalse() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    BrowserOption a(path + QStringLiteral("/minimal.desktop"));
    BrowserOption b(path + QStringLiteral("/nodisplay.desktop"));
    QVERIFY(!(a == b));
}

void BrowserOptionTest::operatorEquals_differentProfile_returnsFalse() {
    const QString path =
        QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR) + QStringLiteral("/minimal.desktop");
    BrowserOption a(path, QStringLiteral("Profile 1"), QString(), false);
    BrowserOption b(path, QStringLiteral("Profile 2"), QString(), false);
    QVERIFY(!(a == b));
}

QTEST_MAIN(BrowserOptionTest)
#include "browseroptiontest.moc"
