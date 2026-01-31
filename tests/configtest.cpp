#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtTest/QTest>

#include "backend.h"
#include "browseroption.h"
#include "config.h"
#include "testdatadir.h"

class ConfigTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void exactMatch_winsOverWildcard();
    void wildcardMatch_subdomain();
    void caseInsensitive_exactMatch();
    void getRememberedBrowser_emptyDomain_returnsEmptyDomain();
    void getRememberedBrowser_emptyValue_returnsInvalidPath();
    void getRememberedBrowser_tildeOnly_returnsInvalidPath();
    void getRememberedBrowser_tildePath_expandsAndReturnsOption();
    void getRememberedBrowser_invalidPath_returnsInvalidPath();
    void getRememberedBrowser_noMatch_returnsNotFound();
    void getRememberedBrowser_noWildcardKey_skippedInLoop();
    void remember_thenGet_returnsOption();
    void remember_withProfile_storesProfileInValue();
    void remember_emptyDomain_doesNothing();
    void forget_removesDomain();
    void forget_emptyDomain_doesNothing();
    void appConfig_hiddenBrowsers();
    void appConfig_includeNoDisplayBrowsers();
    void appConfig_hideGuestProfiles();
    void appConfig_showGuestProfiles();
    void appConfig_hideBrowsersWithoutProfiles();
    void appConfig_rememberChoiceChecked();
    void appConfig_rememberDomainWildcard();
};

static QString s_tempConfigPath;
static QString s_tildeTestHome;
static QByteArray s_oldHome;

static const char *s_minimalDesktopContent = "[Desktop Entry]\n"
                                             "Type=Application\n"
                                             "Name=Minimal Browser\n"
                                             "Exec=/usr/bin/minimal-browser %u\n"
                                             "Icon=minimal-browser\n";

void ConfigTest::initTestCase() {
    s_tempConfigPath =
        QDir::temp().absoluteFilePath(QStringLiteral("browserchooser_configtest.ini"));
    s_oldHome = qgetenv("HOME");
    s_tildeTestHome =
        QDir::temp().absoluteFilePath(QStringLiteral("browserchooser_configtest_home"));
    QVERIFY(QDir().mkpath(s_tildeTestHome));
    qputenv("HOME", s_tildeTestHome.toUtf8());
    const auto tildeDesktopPath =
        s_tildeTestHome + QStringLiteral("/browserchooser_test_minimal.desktop");
    QFile tildeFile(tildeDesktopPath);
    QVERIFY(tildeFile.open(QIODevice::WriteOnly | QIODevice::Text));
    tildeFile.write(QByteArray(s_minimalDesktopContent));
    tildeFile.close();
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto exactPath = testDataDir + QStringLiteral("/minimal.desktop");
    const auto wildcardPath = testDataDir + QStringLiteral("/nodisplay.desktop");
    QFile f(s_tempConfigPath);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&f);
    out << "[RememberedBrowsers]\n";
    out << "a.github.io=" << exactPath << "\n";
    out << "*.github.io=" << wildcardPath << "\n";
    out << "nopattern=" << exactPath << "\n";
    out << "emptydomain=\n";
    out << "tildeonly=~\n";
    out << "baddomain=/nonexistent.desktop\n";
    out << "tildedomain=~/browserchooser_test_minimal.desktop\n";
    f.close();
    setConfigFilePathOverride(s_tempConfigPath);
}

void ConfigTest::cleanupTestCase() {
    clearConfigFilePathOverride();
    QFile::remove(s_tempConfigPath);
    if (!s_tildeTestHome.isEmpty()) {
        QFile::remove(s_tildeTestHome + QStringLiteral("/browserchooser_test_minimal.desktop"));
        QDir().rmpath(s_tildeTestHome);
    }
    if (!s_oldHome.isEmpty()) {
        qputenv("HOME", s_oldHome);
    } else {
        qunsetenv("HOME");
    }
}

void ConfigTest::exactMatch_winsOverWildcard() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("a.github.io"));
    QVERIFY(result.has_value());
    QVERIFY(result->desktopPath().endsWith(QStringLiteral("minimal.desktop")));
}

void ConfigTest::wildcardMatch_subdomain() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("b.github.io"));
    QVERIFY(result.has_value());
    QVERIFY(result->desktopPath().endsWith(QStringLiteral("nodisplay.desktop")));
}

void ConfigTest::caseInsensitive_exactMatch() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("A.GITHUB.IO"));
    QVERIFY(result.has_value());
    QVERIFY(result->desktopPath().endsWith(QStringLiteral("minimal.desktop")));
}

void ConfigTest::getRememberedBrowser_emptyDomain_returnsEmptyDomain() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QString());
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), GetRememberedBrowserError::EmptyDomain);
}

void ConfigTest::getRememberedBrowser_emptyValue_returnsInvalidPath() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("emptydomain"));
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), GetRememberedBrowserError::InvalidPath);
}

void ConfigTest::getRememberedBrowser_tildeOnly_returnsInvalidPath() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("tildeonly"));
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), GetRememberedBrowserError::InvalidPath);
}

void ConfigTest::getRememberedBrowser_tildePath_expandsAndReturnsOption() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("tildedomain"));
    QVERIFY(result.has_value());
    QVERIFY(result->desktopPath().endsWith(QStringLiteral("browserchooser_test_minimal.desktop")));
}

void ConfigTest::getRememberedBrowser_invalidPath_returnsInvalidPath() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("baddomain"));
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), GetRememberedBrowserError::InvalidPath);
}

void ConfigTest::getRememberedBrowser_noMatch_returnsNotFound() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("nonexistent.domain"));
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), GetRememberedBrowserError::NotFound);
}

void ConfigTest::getRememberedBrowser_noWildcardKey_skippedInLoop() {
    SavedBrowsers saved;
    const auto result = saved.getRememberedBrowser(QStringLiteral("nomatch.com"));
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), GetRememberedBrowserError::NotFound);
}

void ConfigTest::remember_thenGet_returnsOption() {
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto desktopPath = testDataDir + QStringLiteral("/minimal.desktop");
    BrowserOption option(desktopPath);
    SavedBrowsers saved;
    saved.remember(QStringLiteral("remembered.com"), option);
    const auto result = saved.getRememberedBrowser(QStringLiteral("remembered.com"));
    QVERIFY(result.has_value());
    QCOMPARE(result->desktopPath(), desktopPath);
}

void ConfigTest::remember_withProfile_storesProfileInValue() {
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto desktopPath = testDataDir + QStringLiteral("/minimal.desktop");
    BrowserOption option(desktopPath, QStringLiteral("Profile 1"), QStringLiteral("Work"), false);
    SavedBrowsers saved;
    saved.remember(QStringLiteral("withprofile.com"), option);
    const auto result = saved.getRememberedBrowser(QStringLiteral("withprofile.com"));
    QVERIFY(result.has_value());
    QCOMPARE(result->desktopPath(), desktopPath);
    QCOMPARE(result->profileName(), QStringLiteral("Profile 1"));
}

void ConfigTest::remember_emptyDomain_doesNothing() {
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    BrowserOption option(testDataDir + QStringLiteral("/minimal.desktop"));
    SavedBrowsers saved;
    saved.remember(QString(), option);
    const auto result = saved.getRememberedBrowser(QStringLiteral("any.domain"));
    QVERIFY(!result.has_value());
}

void ConfigTest::forget_removesDomain() {
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto desktopPath = testDataDir + QStringLiteral("/minimal.desktop");
    BrowserOption option(desktopPath);
    SavedBrowsers saved;
    saved.remember(QStringLiteral("forget.com"), option);
    saved.forget(QStringLiteral("forget.com"));
    const auto result = saved.getRememberedBrowser(QStringLiteral("forget.com"));
    QVERIFY(!result.has_value());
    QCOMPARE(result.error(), GetRememberedBrowserError::NotFound);
}

void ConfigTest::forget_emptyDomain_doesNothing() {
    SavedBrowsers saved;
    saved.forget(QString());
}

void ConfigTest::appConfig_hiddenBrowsers() {
    AppConfig app;
    auto list = app.getHiddenBrowsers();
    QVERIFY(list.isEmpty());
}

void ConfigTest::appConfig_includeNoDisplayBrowsers() {
    AppConfig app;
    QCOMPARE(app.includeNoDisplayBrowsers(), IncludeNoDisplay::No);
}

void ConfigTest::appConfig_hideGuestProfiles() {
    AppConfig app;
    QVERIFY(app.hideGuestProfiles());
    app.setHideGuestProfiles(false);
    QVERIFY(!app.hideGuestProfiles());
}

void ConfigTest::appConfig_showGuestProfiles() {
    AppConfig app;
    app.setShowGuestProfiles(true);
    QVERIFY(app.showGuestProfiles());
    app.setShowGuestProfiles(false);
    QVERIFY(!app.showGuestProfiles());
}

void ConfigTest::appConfig_hideBrowsersWithoutProfiles() {
    AppConfig app;
    QVERIFY(!app.hideBrowsersWithoutProfiles());
    app.setHideBrowsersWithoutProfiles(true);
    QVERIFY(app.hideBrowsersWithoutProfiles());
}

void ConfigTest::appConfig_rememberChoiceChecked() {
    AppConfig app;
    QVERIFY(app.rememberChoiceChecked());
    app.setRememberChoiceChecked(false);
    QVERIFY(!app.rememberChoiceChecked());
}

void ConfigTest::appConfig_rememberDomainWildcard() {
    AppConfig app;
    QVERIFY(app.rememberDomainWildcard());
    app.setRememberDomainWildcard(false);
    QVERIFY(!app.rememberDomainWildcard());
}

QTEST_MAIN(ConfigTest)
#include "configtest.moc"
