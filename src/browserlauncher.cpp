#include <QtCore/QProcess>

#include "browserlauncher.h"

static QStringList parseExecString(const QString &exec) {
    QStringList result;
    QString current;
    auto inQuote = false;
    QChar quoteChar;
    for (auto i = 0; i < exec.length(); ++i) {
        auto c = exec[i];
        if (!inQuote && (c == QLatin1Char('"') || c == QLatin1Char('\''))) {
            inQuote = true;
            quoteChar = c;
        } else if (inQuote && c == quoteChar) {
            inQuote = false;
        } else if (!inQuote && c == QLatin1Char(' ')) {
            if (!current.isEmpty()) {
                result.append(current);
                current.clear();
            }
        } else {
            current.append(c);
        }
    }
    if (!current.isEmpty()) {
        result.append(current);
    }
    return result;
}

void launchBrowser(const DesktopEntry &entry, const QStringList &urls) {
    auto argv = parseExecString(entry.exec());
    if (argv.isEmpty()) {
        return;
    }
    // Handle URL substitution
    QList<QStringList> commands;
    auto uIndex = argv.indexOf(QStringLiteral("%U"));
    auto uLowerIndex = argv.indexOf(QStringLiteral("%u"));
    if (uIndex != -1) {
        // %U allows a list of URLs to be substituted
        QStringList cmd;
        for (auto i = 0; i < argv.size(); ++i) {
            if (i == uIndex) {
                cmd.append(urls);
            } else {
                cmd.append(argv[i]);
            }
        }
        commands.append(cmd);
    } else if (uLowerIndex != -1) {
        // %u allows only a single URL - multiple URLs means multiple launches
        if (urls.isEmpty()) {
            QStringList cmd;
            for (auto i = 0; i < argv.size(); ++i) {
                if (i != uLowerIndex) {
                    cmd.append(argv[i]);
                }
            }
            commands.append(cmd);
        } else {
            for (const auto &url : urls) {
                QStringList cmd;
                for (auto i = 0; i < argv.size(); ++i) {
                    if (i == uLowerIndex) {
                        cmd.append(url);
                    } else {
                        cmd.append(argv[i]);
                    }
                }
                commands.append(cmd);
            }
        }
    } else {
        // No URL placeholder - just run the command
        // Remove other desktop file placeholders like %f, %F, etc.
        QStringList cmd;
        for (const auto &arg : argv) {
            if (!arg.startsWith(QLatin1Char('%'))) {
                cmd.append(arg);
            }
        }
        if (!urls.isEmpty()) {
            cmd.append(urls);
        }
        commands.append(cmd);
    }
    // Launch each command
    for (const auto &cmd : commands) {
        if (cmd.isEmpty()) {
            continue;
        }
        auto program = cmd.first();
        auto args = cmd.mid(1);
        QProcess::startDetached(program, args);
    }
}
