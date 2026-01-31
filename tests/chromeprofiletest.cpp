#include <QtTest/QTest>

#include "chromeprofile.h"
#include "testdatadir.h"

class ChromeProfileTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void getDisplayName_Default();
    void getDisplayName_Profile1();
    void getDisplayName_missingProfile_returnsEmpty();
    void getDisplayName_missingFile_returnsEmpty();
    void getDisplayName_invalidJson_returnsEmpty();
    void getDisplayName_emptyName_returnsKey();
    void getPicturePath_Default_returnsPath();
    void getPicturePath_Profile1_returnsPath();
    void getPicturePath_missingProfile_returnsEmpty();
    void getPicturePath_emptyFileName_returnsEmpty();
    void getPicturePath_fileNotExists_returnsEmpty();
    void getPicturePath_invalidJson_returnsEmpty();
    void getPicturePath_missingLocalState_returnsEmpty();
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

void ChromeProfileTest::getDisplayName_invalidJson_returnsEmpty() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome_invalid_json");
    const auto name =
        getChromeProfileDisplayNameFromUserDataDir(userDataDir, QStringLiteral("Default"));
    QVERIFY(name.isEmpty());
}

void ChromeProfileTest::getDisplayName_emptyName_returnsKey() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome_empty_name");
    const auto name =
        getChromeProfileDisplayNameFromUserDataDir(userDataDir, QStringLiteral("Profile 2"));
    QCOMPARE(name, QStringLiteral("Profile 2"));
}

void ChromeProfileTest::getPicturePath_Default_returnsPath() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome");
    const auto picturePath =
        getChromeProfilePicturePathFromUserDataDir(userDataDir, QStringLiteral("Default"));
    QVERIFY(!picturePath.isEmpty());
    QVERIFY(picturePath.endsWith(QStringLiteral("/Default/avatar.png")));
}

void ChromeProfileTest::getPicturePath_Profile1_returnsPath() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome");
    const auto picturePath =
        getChromeProfilePicturePathFromUserDataDir(userDataDir, QStringLiteral("Profile 1"));
    QVERIFY(!picturePath.isEmpty());
    QVERIFY(picturePath.endsWith(QStringLiteral("/Profile 1/work_avatar.png")));
}

void ChromeProfileTest::getPicturePath_missingProfile_returnsEmpty() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome");
    const auto picturePath =
        getChromeProfilePicturePathFromUserDataDir(userDataDir, QStringLiteral("Profile 99"));
    QVERIFY(picturePath.isEmpty());
}

void ChromeProfileTest::getPicturePath_emptyFileName_returnsEmpty() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome_empty_name");
    const auto picturePath =
        getChromeProfilePicturePathFromUserDataDir(userDataDir, QStringLiteral("Profile 2"));
    QVERIFY(picturePath.isEmpty());
}

void ChromeProfileTest::getPicturePath_fileNotExists_returnsEmpty() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome_picture_missing");
    const auto picturePath =
        getChromeProfilePicturePathFromUserDataDir(userDataDir, QStringLiteral("Default"));
    QVERIFY(picturePath.isEmpty());
}

void ChromeProfileTest::getPicturePath_invalidJson_returnsEmpty() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto userDataDir = path + QStringLiteral("/chrome_invalid_json");
    const auto picturePath =
        getChromeProfilePicturePathFromUserDataDir(userDataDir, QStringLiteral("Default"));
    QVERIFY(picturePath.isEmpty());
}

void ChromeProfileTest::getPicturePath_missingLocalState_returnsEmpty() {
    const auto picturePath =
        getChromeProfilePicturePathFromUserDataDir(QStringLiteral("/nonexistent"), QString());
    QVERIFY(picturePath.isEmpty());
}

QTEST_MAIN(ChromeProfileTest)
#include "chromeprofiletest.moc"
