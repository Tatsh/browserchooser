#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>

#include "browserlauncher.h"

namespace {

QStringList parseExecString(const QString &exec) {
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

bool isProfileRelatedArg(const QString &arg) {
    return arg == QLatin1String("-P") || arg == QLatin1String("--guest") ||
           arg.startsWith(QLatin1String("--profile="));
}

QString quoteArg(const QString &arg) {
    if (arg.isEmpty()) {
        return QStringLiteral(R"("")");
    }
    auto needQuote = false;
    for (auto c : arg) {
        if (c == QLatin1Char(' ') || c == QLatin1Char('"') || c == QLatin1Char('\'') ||
            c == QLatin1Char('\\')) {
            needQuote = true;
            break;
        }
    }
    if (!needQuote) {
        return arg;
    }
    QString result;
    result.reserve(arg.size() + 2);
    result += QLatin1Char('"');
    for (auto c : arg) {
        if (c == QLatin1Char('"')) {
            result += QStringLiteral(R"(\")");
        } else if (c == QLatin1Char('\\')) {
            result += QStringLiteral(R"(\\)");
        } else {
            result += c;
        }
    }
    result += QLatin1Char('"');
    return result;
}

QStringList buildArgvForOption(const BrowserOption &option, const QString &url) {
    auto entry = option.entry();
    if (!entry.isValid()) {
        return {};
    }
    auto argv = parseExecString(entry.exec());
    if (argv.isEmpty()) {
        return {};
    }
    QStringList cmd;
    auto uIndex = argv.indexOf(QStringLiteral("%U"));
    auto uLowerIndex = argv.indexOf(QStringLiteral("%u"));
    if (uIndex != -1) {
        for (auto i = 0; i < argv.size(); ++i) {
            if (i == uIndex) {
                if (!url.isEmpty()) {
                    cmd.append(url);
                }
            } else if (argv[i] == QLatin1String("-P")) {
                ++i; // skip -P and its value
            } else if (!isProfileRelatedArg(argv[i])) {
                cmd.append(argv[i]);
            }
        }
    } else if (uLowerIndex != -1) {
        for (auto i = 0; i < argv.size(); ++i) {
            if (i == uLowerIndex) {
                continue; // URL appended below.
            }
            if (argv[i] == QLatin1String("-P")) {
                ++i;
                continue;
            }
            if (!isProfileRelatedArg(argv[i])) {
                cmd.append(argv[i]);
            }
        }
        if (!url.isEmpty()) {
            cmd.append(url);
        }
    } else {
        for (auto i = 0; i < argv.size(); ++i) {
            if (argv[i] == QLatin1String("-P")) {
                ++i; // skip -P and its value
                continue;
            }
            if (isProfileRelatedArg(argv[i]) || argv[i].startsWith(QLatin1Char('%'))) {
                continue;
            }
            cmd.append(argv[i]);
        }
        if (!url.isEmpty()) {
            cmd.append(url);
        }
    }
    if (cmd.isEmpty()) {
        return {};
    }
    auto program = cmd.first();
    auto args = cmd.mid(1);
    if (!option.profileName().isEmpty()) {
        auto exeName = option.entry().executableName();
        if (exeName.contains(QStringLiteral("firefox"), Qt::CaseInsensitive)) {
            args.prepend(option.profileName());
            args.prepend(QStringLiteral("-P"));
        } else if (option.profileName() == QStringLiteral("Guest")) {
            args.prepend(QStringLiteral("--guest"));
        } else {
            args.prepend(QStringLiteral("--profile=") + option.profileName());
        }
    }
    QStringList result;
    result.append(program);
    result.append(args);
    return result;
}

} // anonymous namespace

void launchBrowser(const BrowserOption &option, const QStringList &urls) {
    auto entry = option.entry();
    if (!entry.isValid()) {
        return;
    }
    auto argv = parseExecString(entry.exec());
    if (argv.isEmpty()) {
        return;
    }
    // Handle URL substitution.
    QList<QStringList> commands;
    auto uIndex = argv.indexOf(QStringLiteral("%U"));
    auto uLowerIndex = argv.indexOf(QStringLiteral("%u"));
    if (uIndex != -1) {
        // %U allows a list of URLs to be substituted
        QStringList cmd;
        for (auto i = 0; i < argv.size(); ++i) {
            if (i == uIndex) {
                cmd.append(urls);
            } else if (argv[i] == QLatin1String("-P")) {
                ++i; // skip -P and its value from Exec
            } else if (!isProfileRelatedArg(argv[i])) {
                cmd.append(argv[i]);
            }
        }
        commands.append(cmd);
    } else if (uLowerIndex != -1) {
        // %u allows only a single URL - multiple URLs means multiple launches
        if (urls.isEmpty()) {
            QStringList cmd;
            for (auto i = 0; i < argv.size(); ++i) {
                if (i == uLowerIndex) {
                    continue;
                }
                if (argv[i] == QLatin1String("-P")) {
                    ++i;
                    continue;
                }
                if (!isProfileRelatedArg(argv[i])) {
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
                    } else if (argv[i] == QLatin1String("-P")) {
                        ++i;
                        continue;
                    } else if (!isProfileRelatedArg(argv[i])) {
                        cmd.append(argv[i]);
                    }
                }
                commands.append(cmd);
            }
        }
    } else {
        // No URL placeholder - just run the command.
        QStringList cmd;
        for (auto i = 0; i < argv.size(); ++i) {
            if (argv[i] == QLatin1String("-P")) {
                ++i;
                continue;
            }
            if (isProfileRelatedArg(argv[i]) || argv[i].startsWith(QLatin1Char('%'))) {
                continue;
            }
            cmd.append(argv[i]);
        }
        if (!urls.isEmpty()) {
            cmd.append(urls);
        }
        commands.append(cmd);
    }
    for (auto cmd : commands) {
        if (cmd.isEmpty()) {
            continue;
        }
        auto program = cmd.first();
        auto args = cmd.mid(1);
        if (!option.profileName().isEmpty()) {
            auto exeName = option.entry().executableName();
            if (exeName.contains(QStringLiteral("firefox"), Qt::CaseInsensitive)) {
                args.prepend(option.profileName());
                args.prepend(QStringLiteral("-P"));
            } else if (option.profileName() == QStringLiteral("Guest")) {
                args.prepend(QStringLiteral("--guest"));
            } else {
                args.prepend(QStringLiteral("--profile=") + option.profileName());
            }
        }
        QProcess::startDetached(program, args);
    }
}

QString getCommandLineForDisplay(const BrowserOption &option, const QString &url) {
    auto argv = buildArgvForOption(option, url);
    if (argv.isEmpty()) {
        return {};
    }
    auto program = argv.first();
    auto fullPath = QStandardPaths::findExecutable(program);
    if (!fullPath.isEmpty()) {
        argv[0] = fullPath;
    }
    QStringList quoted;
    for (const auto &arg : argv) {
        quoted.append(quoteArg(arg));
    }
    return quoted.join(QLatin1Char(' '));
}

QString getExecutablePath(const BrowserOption &option) {
    auto entry = option.entry();
    if (!entry.isValid()) {
        return {};
    }
    auto exeName = entry.executableName();
    if (exeName.isEmpty()) {
        return {};
    }
    auto exePath = QStandardPaths::findExecutable(exeName);
    if (exePath.isEmpty() && QFileInfo(exeName).isAbsolute() && QFile::exists(exeName)) {
        exePath = exeName;
    }
    return exePath;
}
