#include <QtTest/QTest>

#include "domainmatch.h"

class DomainMatchTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void wildcard_starPrefix_matchesBaseDomain();
    void wildcard_starPrefix_matchesSubdomain();
    void wildcard_starPrefix_mismatch();
    void wildcard_regexPattern_matches();
    void wildcard_regexPattern_caseInsensitive();
    void wildcard_nonStarPrefix_regexMatch();
    void wildcard_nonStarPrefix_regexMismatch();
};

void DomainMatchTest::wildcard_starPrefix_matchesBaseDomain() {
    QVERIFY(matchesWildcardPattern(QStringLiteral("*.github.io"), QStringLiteral("github.io")));
}

void DomainMatchTest::wildcard_starPrefix_matchesSubdomain() {
    QVERIFY(matchesWildcardPattern(QStringLiteral("*.github.io"), QStringLiteral("a.github.io")));
    QVERIFY(
        matchesWildcardPattern(QStringLiteral("*.google.com"), QStringLiteral("www.google.com")));
}

void DomainMatchTest::wildcard_starPrefix_mismatch() {
    QVERIFY(!matchesWildcardPattern(QStringLiteral("*.github.io"), QStringLiteral("github.com")));
    QVERIFY(!matchesWildcardPattern(QStringLiteral("*.google.com"), QStringLiteral("google.org")));
}

void DomainMatchTest::wildcard_regexPattern_matches() {
    QVERIFY(matchesWildcardPattern(QStringLiteral("*.example.com"),
                                   QStringLiteral("mail.example.com")));
}

void DomainMatchTest::wildcard_regexPattern_caseInsensitive() {
    QVERIFY(matchesWildcardPattern(QStringLiteral("*.GitHub.IO"), QStringLiteral("a.github.io")));
}

void DomainMatchTest::wildcard_nonStarPrefix_regexMatch() {
    // Pattern does not start with "*." so uses QRegularExpression wildcard path (line 27+).
    QVERIFY(matchesWildcardPattern(QStringLiteral("www.*.com"), QStringLiteral("www.google.com")));
}

void DomainMatchTest::wildcard_nonStarPrefix_regexMismatch() {
    // Same path, domain does not match pattern.
    QVERIFY(!matchesWildcardPattern(QStringLiteral("www.*.com"), QStringLiteral("ftp.google.com")));
}

QTEST_MAIN(DomainMatchTest)
#include "domainmatchtest.moc"
