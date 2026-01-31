#include "basedomain.h"

#include <QtCore/QSet>

QString getBaseDomain(const QString &domain) {
    auto parts = domain.split(QLatin1Char('.'));
    if (parts.size() <= 2) {
        return domain;
    }
    // Known two-part public suffixes (multi-part TLDs). When the last two
    // parts form one of these, the base domain is the last three parts.
    static const QSet<QString> kTwoPartSuffixes = {
        QStringLiteral("ac.uk"),   QStringLiteral("asn.au"),   QStringLiteral("co.au"),
        QStringLiteral("co.id"),   QStringLiteral("co.il"),    QStringLiteral("co.in"),
        QStringLiteral("co.jp"),   QStringLiteral("co.kr"),    QStringLiteral("co.nz"),
        QStringLiteral("co.th"),   QStringLiteral("co.uk"),    QStringLiteral("co.za"),
        QStringLiteral("com.au"),  QStringLiteral("com.br"),   QStringLiteral("com.mx"),
        QStringLiteral("ed.jp"),   QStringLiteral("edu.au"),   QStringLiteral("gen.nz"),
        QStringLiteral("go.jp"),   QStringLiteral("gov.uk"),   QStringLiteral("gr.jp"),
        QStringLiteral("id.au"),   QStringLiteral("lg.jp"),    QStringLiteral("ltd.uk"),
        QStringLiteral("me.uk"),   QStringLiteral("ne.jp"),    QStringLiteral("net.au"),
        QStringLiteral("net.br"),  QStringLiteral("net.uk"),   QStringLiteral("or.jp"),
        QStringLiteral("org.au"),  QStringLiteral("org.uk"),   QStringLiteral("ac.jp"),
        QStringLiteral("plc.uk"),  QStringLiteral("sch.uk"),   QStringLiteral("ac.nz"),
        QStringLiteral("gov.au"),  QStringLiteral("govt.nz"),  QStringLiteral("geek.nz"),
        QStringLiteral("kiwi.nz"), QStringLiteral("maori.nz"), QStringLiteral("school.nz"),
        QStringLiteral("net.nz"),  QStringLiteral("org.nz"),   QStringLiteral("com.ar"),
        QStringLiteral("net.in"),  QStringLiteral("org.in"),   QStringLiteral("ac.in"),
        QStringLiteral("edu.in"),  QStringLiteral("gov.in"),   QStringLiteral("res.in"),
        QStringLiteral("gen.in"),  QStringLiteral("firm.in"),  QStringLiteral("ind.in"),
        QStringLiteral("org.za"),  QStringLiteral("web.za"),   QStringLiteral("net.za"),
        QStringLiteral("gov.za"),  QStringLiteral("edu.za"),   QStringLiteral("mil.za"),
        QStringLiteral("ac.za"),   QStringLiteral("law.za"),   QStringLiteral("or.kr"),
        QStringLiteral("go.kr"),   QStringLiteral("ac.kr"),    QStringLiteral("ne.kr"),
        QStringLiteral("re.kr"),   QStringLiteral("org.mx"),   QStringLiteral("gob.mx"),
        QStringLiteral("edu.mx"),  QStringLiteral("net.mx"),   QStringLiteral("web.mx"),
    };
    const auto twoPartSuffix = parts.mid(parts.size() - 2).join(QLatin1Char('.'));
    if (kTwoPartSuffixes.contains(twoPartSuffix)) {
        return parts.mid(parts.size() - 3).join(QLatin1Char('.'));
    }
    return parts.mid(parts.size() - 2).join(QLatin1Char('.'));
}
