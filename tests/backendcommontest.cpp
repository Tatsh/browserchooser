#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtTest/QTest>

#include "backend.h"
#include "browseroption.h"
#include "desktopentry.h"
#include "testdatadir.h"

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
    void sortBrowserOptionsByDisplayName_ordersCaseInsensitive();
    void getPreLaunchCommands_withJson_returnsCommands();
    void getPreLaunchCommands_fallbackToDesktopPathOnly();
    void getPreLaunchCommands_emptyDesktopPath_returnsEmpty();
    void getPreLaunchCommands_noKey_returnsEmpty();
    void getPreLaunchCommands_invalidJson_returnsEmpty();
    void getPostLaunchCommands_withJson_returnsCommands();
    void getChromeProfilePicturePath_withChromeConfigHome_returnsPath();
#ifndef Q_OS_WIN
    void quoteArg_empty_returnsEmptyQuoted();
    void quoteArg_safeChars_returnsUnchanged();
    void quoteArg_withSpace_returnsQuoted();
    void quoteArg_withSingleQuote_returnsEscaped();
    void quoteArg_highByteChar_returnsQuoted();
    void quoteArg_lowercaseOnly_returnsUnchanged();
    void quoteArg_uppercaseOnly_returnsUnchanged();
#endif
    void getPostLaunchCommands_emptyDesktopPath_returnsEmpty();
    void getPostLaunchCommands_fallbackToDesktopPathOnly();
    void getChromeProfileDisplayName_guest_returnsTranslated();
    void getChromeProfileDisplayName_invalidPath_returnsEmpty();
    void getChromeProfileDisplayName_noLocalState_returnsEmpty();
    void getChromeProfileDisplayName_withChromeConfigHome_returnsName();
    void getChromeProfilePicturePath_guest_returnsEmpty();
    void getChromeProfilePicturePath_invalidPath_returnsEmpty();
};

static QString s_tempConfigPath;

void BackendCommonTest::initTestCase() {
    s_tempConfigPath =
        QDir::temp().absoluteFilePath(QStringLiteral("browserchooser_backendtest.ini"));
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    QSettings settings(s_tempConfigPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("SomeGroup/SomeKey"), QStringLiteral("one, two , three"));
    settings.setValue(QStringLiteral("PreLaunchCommands/") + minimalPath,
                      QStringLiteral("[[\"echo\",\"hi\"],\"x\",[\"a\",\"b\"],[\"only\"]]"));
    settings.setValue(QStringLiteral("PostLaunchCommands/") + minimalPath,
                      QStringLiteral("[[\"true\"]]"));
    const QString nodisplayPath = testDataDir + QStringLiteral("/nodisplay.desktop");
    settings.setValue(QStringLiteral("PreLaunchCommands/") + nodisplayPath, QStringLiteral("null"));
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

void BackendCommonTest::sortBrowserOptionsByDisplayName_ordersCaseInsensitive() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    QList<BrowserOption> options;
    options.append(BrowserOption(testDataDir + QStringLiteral("/nodisplay.desktop")));
    options.append(BrowserOption(testDataDir + QStringLiteral("/minimal.desktop")));
    sortBrowserOptionsByDisplayName(options);
    QCOMPARE(options.size(), 2);
    QCOMPARE(options.at(0).displayName(), QStringLiteral("Minimal Browser (Default)"));
    QCOMPARE(options.at(1).displayName(), QStringLiteral("NoDisplay Browser (Default)"));
}

void BackendCommonTest::getPreLaunchCommands_withJson_returnsCommands() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    setConfigFilePathOverride(s_tempConfigPath);
    const auto commands = getPreLaunchCommands(minimalPath, QString());
    clearConfigFilePathOverride();
    QCOMPARE(commands.size(), 3);
    QCOMPARE(commands.at(0), (QStringList{QStringLiteral("echo"), QStringLiteral("hi")}));
    QCOMPARE(commands.at(1), (QStringList{QStringLiteral("a"), QStringLiteral("b")}));
    QCOMPARE(commands.at(2), (QStringList{QStringLiteral("only")}));
}

void BackendCommonTest::getPreLaunchCommands_fallbackToDesktopPathOnly() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    setConfigFilePathOverride(s_tempConfigPath);
    const auto commands = getPreLaunchCommands(minimalPath, QStringLiteral("Profile 1"));
    clearConfigFilePathOverride();
    QCOMPARE(commands.size(), 3);
    QCOMPARE(commands.at(0), (QStringList{QStringLiteral("echo"), QStringLiteral("hi")}));
}

void BackendCommonTest::getPreLaunchCommands_emptyDesktopPath_returnsEmpty() {
    const auto commands = getPreLaunchCommands(QString(), QString());
    QVERIFY(commands.isEmpty());
}

void BackendCommonTest::getPreLaunchCommands_noKey_returnsEmpty() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString path = testDataDir + QStringLiteral("/categories.desktop");
    setConfigFilePathOverride(s_tempConfigPath);
    const auto commands = getPreLaunchCommands(path, QString());
    clearConfigFilePathOverride();
    QVERIFY(commands.isEmpty());
}

void BackendCommonTest::getPreLaunchCommands_invalidJson_returnsEmpty() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString nodisplayPath = testDataDir + QStringLiteral("/nodisplay.desktop");
    setConfigFilePathOverride(s_tempConfigPath);
    const auto commands = getPreLaunchCommands(nodisplayPath, QString());
    clearConfigFilePathOverride();
    QVERIFY(commands.isEmpty());
}

void BackendCommonTest::getPostLaunchCommands_withJson_returnsCommands() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    setConfigFilePathOverride(s_tempConfigPath);
    const auto commands = getPostLaunchCommands(minimalPath, QString());
    clearConfigFilePathOverride();
    QCOMPARE(commands.size(), 1);
    QCOMPARE(commands.at(0), (QStringList{QStringLiteral("true")}));
}

void BackendCommonTest::getChromeProfilePicturePath_withChromeConfigHome_returnsPath() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    qputenv("CHROME_CONFIG_HOME", testDataDir.toUtf8());
    const auto path = getChromeProfilePicturePath(minimalPath, QStringLiteral("Default"));
    qunsetenv("CHROME_CONFIG_HOME");
    QVERIFY(!path.isEmpty());
    QVERIFY(path.endsWith(QStringLiteral("/Default/avatar.png")));
}

#ifndef Q_OS_WIN
void BackendCommonTest::quoteArg_empty_returnsEmptyQuoted() {
    QCOMPARE(quoteArg(QString()), QStringLiteral("''"));
}

void BackendCommonTest::quoteArg_safeChars_returnsUnchanged() {
    QCOMPARE(quoteArg(QStringLiteral("abc123")), QStringLiteral("abc123"));
    QCOMPARE(quoteArg(QStringLiteral("foo-bar")), QStringLiteral("foo-bar"));
}

void BackendCommonTest::quoteArg_withSpace_returnsQuoted() {
    QCOMPARE(quoteArg(QStringLiteral("a b")), QStringLiteral("'a b'"));
}

void BackendCommonTest::quoteArg_withSingleQuote_returnsEscaped() {
    QCOMPARE(quoteArg(QStringLiteral("a'b")), QStringLiteral("'a'\"'\"'b'"));
}

void BackendCommonTest::quoteArg_highByteChar_returnsQuoted() {
    const QString arg = QString::fromUtf8("caf\xc3\xa9");
    const QString expected = QLatin1Char('\'') + arg + QLatin1Char('\'');
    QCOMPARE(quoteArg(arg), expected);
}

void BackendCommonTest::quoteArg_lowercaseOnly_returnsUnchanged() {
    QCOMPARE(quoteArg(QStringLiteral("abcdef")), QStringLiteral("abcdef"));
}

void BackendCommonTest::quoteArg_uppercaseOnly_returnsUnchanged() {
    QCOMPARE(quoteArg(QStringLiteral("ABCXYZ")), QStringLiteral("ABCXYZ"));
}
#endif

void BackendCommonTest::getPostLaunchCommands_emptyDesktopPath_returnsEmpty() {
    const auto commands = getPostLaunchCommands(QString(), QString());
    QVERIFY(commands.isEmpty());
}

void BackendCommonTest::getPostLaunchCommands_fallbackToDesktopPathOnly() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    setConfigFilePathOverride(s_tempConfigPath);
    const auto commands = getPostLaunchCommands(minimalPath, QStringLiteral("Profile 1"));
    clearConfigFilePathOverride();
    QCOMPARE(commands.size(), 1);
    QCOMPARE(commands.at(0), (QStringList{QStringLiteral("true")}));
}

void BackendCommonTest::getChromeProfileDisplayName_guest_returnsTranslated() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    QCOMPARE(getChromeProfileDisplayName(minimalPath, QStringLiteral("Guest")),
             QCoreApplication::translate("BrowserChooser", "Guest"));
}

void BackendCommonTest::getChromeProfileDisplayName_invalidPath_returnsEmpty() {
    QVERIFY(getChromeProfileDisplayName(QStringLiteral("/nonexistent/file.desktop"),
                                        QStringLiteral("Default"))
                .isEmpty());
}

void BackendCommonTest::getChromeProfileDisplayName_noLocalState_returnsEmpty() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString nodisplayPath = testDataDir + QStringLiteral("/nodisplay.desktop");
    qputenv("CHROME_CONFIG_HOME", testDataDir.toUtf8());
    QVERIFY(getChromeProfileDisplayName(nodisplayPath, QStringLiteral("Default")).isEmpty());
    qunsetenv("CHROME_CONFIG_HOME");
}

void BackendCommonTest::getChromeProfileDisplayName_withChromeConfigHome_returnsName() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    qputenv("CHROME_CONFIG_HOME", testDataDir.toUtf8());
    QCOMPARE(getChromeProfileDisplayName(minimalPath, QStringLiteral("Default")),
             QStringLiteral("Person 1"));
    qunsetenv("CHROME_CONFIG_HOME");
}

void BackendCommonTest::getChromeProfilePicturePath_guest_returnsEmpty() {
    const QString testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const QString minimalPath = testDataDir + QStringLiteral("/minimal.desktop");
    QVERIFY(getChromeProfilePicturePath(minimalPath, QStringLiteral("Guest")).isEmpty());
}

void BackendCommonTest::getChromeProfilePicturePath_invalidPath_returnsEmpty() {
    QVERIFY(getChromeProfilePicturePath(QStringLiteral("/nonexistent/file.desktop"),
                                        QStringLiteral("Default"))
                .isEmpty());
}

QTEST_MAIN(BackendCommonTest)
#include "backendcommontest.moc"
