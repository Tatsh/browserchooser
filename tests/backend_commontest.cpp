#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtTest/QTest>

#include "backend.h"
#include "browseroption.h"
#include "desktopentry.h"
#include "testdata_dir.h"

class BackendCommonTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void listContainsIdentifier_present();
    void listContainsIdentifier_absent();
    void listContainsIdentifier_caseInsensitive();
    void listContainsIdentifier_emptyList_returnsFalse();
    void listContainsIdentifier_emptyId_returnsFalse();
    void isBrowserHidden_baseNameInList_returnsTrue();
    void isBrowserHidden_baseNameNotInList_returnsFalse();
    void isBrowserHidden_emptyList_returnsFalse();
    void readCommaSeparatedList_withTempConfig();
};

static QString s_tempConfigPath;

void BackendCommonTest::initTestCase() {
    s_tempConfigPath =
        QDir::temp().absoluteFilePath(QStringLiteral("browserchooser_backendtest.ini"));
    QSettings settings(s_tempConfigPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("SomeGroup/SomeKey"), QStringLiteral("one, two , three"));
    settings.sync();
}

void BackendCommonTest::cleanupTestCase() {
    clearConfigFilePathOverride();
    QFile::remove(s_tempConfigPath);
}

void BackendCommonTest::listContainsIdentifier_present() {
    QStringList list{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")};
    QVERIFY(listContainsIdentifier(list, QStringLiteral("b")));
}

void BackendCommonTest::listContainsIdentifier_absent() {
    QStringList list{QStringLiteral("a"), QStringLiteral("b")};
    QVERIFY(!listContainsIdentifier(list, QStringLiteral("x")));
}

void BackendCommonTest::listContainsIdentifier_caseInsensitive() {
    QStringList list{QStringLiteral("Chrome")};
    QVERIFY(listContainsIdentifier(list, QStringLiteral("chrome")));
}

void BackendCommonTest::listContainsIdentifier_emptyList_returnsFalse() {
    QVERIFY(!listContainsIdentifier(QStringList(), QStringLiteral("a")));
}

void BackendCommonTest::listContainsIdentifier_emptyId_returnsFalse() {
    QStringList list{QStringLiteral("a")};
    QVERIFY(!listContainsIdentifier(list, QString()));
}

void BackendCommonTest::isBrowserHidden_baseNameInList_returnsTrue() {
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto desktopPath = testDataDir + QStringLiteral("/minimal.desktop");
    BrowserOption option(desktopPath);
    QStringList hidden{QStringLiteral("minimal")};
    QVERIFY(isBrowserHidden(option, hidden));
}

void BackendCommonTest::isBrowserHidden_baseNameNotInList_returnsFalse() {
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto desktopPath = testDataDir + QStringLiteral("/minimal.desktop");
    BrowserOption option(desktopPath);
    QStringList hidden{QStringLiteral("other")};
    QVERIFY(!isBrowserHidden(option, hidden));
}

void BackendCommonTest::isBrowserHidden_emptyList_returnsFalse() {
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto desktopPath = testDataDir + QStringLiteral("/minimal.desktop");
    BrowserOption option(desktopPath);
    QVERIFY(!isBrowserHidden(option, QStringList()));
}

void BackendCommonTest::readCommaSeparatedList_withTempConfig() {
    setConfigFilePathOverride(s_tempConfigPath);
    const auto list = readCommaSeparatedList(QStringLiteral("SomeGroup/SomeKey"));
    clearConfigFilePathOverride();
    QCOMPARE(list.size(), 3);
    QCOMPARE(list.at(0), QStringLiteral("one"));
    QCOMPARE(list.at(1), QStringLiteral("two"));
    QCOMPARE(list.at(2), QStringLiteral("three"));
}

QTEST_MAIN(BackendCommonTest)
#include "backend_commontest.moc"
