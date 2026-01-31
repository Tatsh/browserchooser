#include <QtTest/QTest>

#include "basedomain.h"

class BaseDomainTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void twoPartSuffix_returnsAsIs();
    void threePart_withTwoPartTld_returnsThreeParts();
    void twoPart_returnsTwoParts();
    void singleLabel_returnsAsIs();
};

void BaseDomainTest::twoPartSuffix_returnsAsIs() {
    // github.io is not in two-part suffix list, so a.github.io → github.io (last 2 parts).
    QCOMPARE(getBaseDomain(QStringLiteral("a.github.io")), QStringLiteral("github.io"));
    QCOMPARE(getBaseDomain(QStringLiteral("example.com")), QStringLiteral("example.com"));
}

void BaseDomainTest::threePart_withTwoPartTld_returnsThreeParts() {
    QCOMPARE(getBaseDomain(QStringLiteral("www.amazon.co.uk")), QStringLiteral("amazon.co.uk"));
}

void BaseDomainTest::twoPart_returnsTwoParts() {
    QCOMPARE(getBaseDomain(QStringLiteral("example.com")), QStringLiteral("example.com"));
    QCOMPARE(getBaseDomain(QStringLiteral("sub.example.com")), QStringLiteral("example.com"));
}

void BaseDomainTest::singleLabel_returnsAsIs() {
    QCOMPARE(getBaseDomain(QStringLiteral("localhost")), QStringLiteral("localhost"));
}

QTEST_MAIN(BaseDomainTest)
#include "basedomaintest.moc"
