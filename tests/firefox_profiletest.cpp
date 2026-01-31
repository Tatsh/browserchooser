#include <QtTest/QTest>

#include "firefox_profile.h"
#include "testdata_dir.h"

class FirefoxProfileTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void getFirefoxProfiles_twoProfiles_returnsDefaultAndNamed();
    void getFirefoxProfiles_missingFile_returnsEmpty();
};

void FirefoxProfileTest::getFirefoxProfiles_twoProfiles_returnsDefaultAndNamed() {
    const QString path = QString::fromUtf8(BROWSERCHOOSER_TEST_DATA_DIR_STR);
    const auto configDir = path + QStringLiteral("/firefox");
    const auto pairs = getFirefoxProfiles(configDir);
    QVERIFY(pairs.size() >= 2);
    QVERIFY(pairs.first().first.isEmpty());
    QVERIFY(pairs.first().second.isEmpty());
    bool hasWork = false;
    for (const auto &p : pairs) {
        if (p.second == QStringLiteral("Work")) {
            hasWork = true;
            break;
        }
    }
    QVERIFY(hasWork);
}

void FirefoxProfileTest::getFirefoxProfiles_missingFile_returnsEmpty() {
    const auto pairs = getFirefoxProfiles(QStringLiteral("/nonexistent"));
    QVERIFY(pairs.isEmpty());
}

QTEST_MAIN(FirefoxProfileTest)
#include "firefox_profiletest.moc"
