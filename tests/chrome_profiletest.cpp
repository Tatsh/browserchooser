#include <QtTest/QTest>

#include "chrome_profile.h"
#include "testdata_dir.h"

class ChromeProfileTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void getDisplayName_Default();
    void getDisplayName_Profile1();
    void getDisplayName_missingProfile_returnsEmpty();
    void getDisplayName_missingFile_returnsEmpty();
};

void ChromeProfileTest::getDisplayName_Default() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome");
    const auto name =
        getChromeProfileDisplayNameFromUserDataDir(userDataDir, QStringLiteral("Default"));
    QCOMPARE(name, QStringLiteral("Person 1"));
}

void ChromeProfileTest::getDisplayName_Profile1() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome");
    const auto name =
        getChromeProfileDisplayNameFromUserDataDir(userDataDir, QStringLiteral("Profile 1"));
    QCOMPARE(name, QStringLiteral("Work"));
}

void ChromeProfileTest::getDisplayName_missingProfile_returnsEmpty() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome");
    const auto name =
        getChromeProfileDisplayNameFromUserDataDir(userDataDir, QStringLiteral("Profile 99"));
    QVERIFY(name.isEmpty());
}

void ChromeProfileTest::getDisplayName_missingFile_returnsEmpty() {
    const auto name =
        getChromeProfileDisplayNameFromUserDataDir(QStringLiteral("/nonexistent"), QString());
    QVERIFY(name.isEmpty());
}

QTEST_MAIN(ChromeProfileTest)
#include "chrome_profiletest.moc"
