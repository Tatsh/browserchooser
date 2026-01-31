#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtTest/QTest>

#include "backend.h"
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
};

static QString s_tempConfigPath;

void ConfigTest::initTestCase() {
    s_tempConfigPath =
        QDir::temp().absoluteFilePath(QStringLiteral("browserchooser_configtest.ini"));
    const auto testDataDir = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto exactPath = testDataDir + QStringLiteral("/minimal.desktop");
    const auto wildcardPath = testDataDir + QStringLiteral("/nodisplay.desktop");
    QFile f(s_tempConfigPath);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&f);
    out << "[RememberedBrowsers]\n";
    out << "a.github.io=" << exactPath << "\n";
    out << "*.github.io=" << wildcardPath << "\n";
    f.close();
    setConfigFilePathOverride(s_tempConfigPath);
}

void ConfigTest::cleanupTestCase() {
    clearConfigFilePathOverride();
    QFile::remove(s_tempConfigPath);
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

QTEST_MAIN(ConfigTest)
#include "configtest.moc"
