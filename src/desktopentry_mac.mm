#include <QtCore/QFile>

#import <Foundation/Foundation.h>

#include "desktopentry.h"

static QString stringFromNSString(NSString *ns) {
    if (!ns || ![ns isKindOfClass:[NSString class]]) {
        return {};
    }
    return QString::fromUtf8([ns UTF8String]);
}

static NSDictionary *loadPlist(const QString &plistPath) {
    const QByteArray pathUtf8 = plistPath.toUtf8();
    NSString *path = [NSString stringWithUTF8String:pathUtf8.constData()];
    return [NSDictionary dictionaryWithContentsOfFile:path];
}

bool DesktopEntry::parseAppBundle(const QString &bundlePath) {
    valid_ = false;
    filename_ = bundlePath;
    entries_.clear();
    const auto plistPath = bundlePath + QStringLiteral("/Contents/Info.plist");
    if (!QFile::exists(plistPath)) {
        return false;
    }
    @autoreleasepool {
        NSDictionary *root = loadPlist(plistPath);
        if (!root) {
            return false;
        }
        NSString *execNameNs = root[@"CFBundleExecutable"];
        if (!execNameNs || ![execNameNs isKindOfClass:[NSString class]] ||
            [execNameNs length] == 0) {
            return false;
        }
        QString execName = stringFromNSString(execNameNs);
        exec_ = bundlePath + QStringLiteral("/Contents/MacOS/") + execName;
        if (!QFile::exists(exec_)) {
            return false;
        }
        NSString *nameNs = root[@"CFBundleDisplayName"];
        if (!nameNs || ![nameNs isKindOfClass:[NSString class]] || [nameNs length] == 0) {
            nameNs = root[@"CFBundleName"];
        }
        if (!nameNs || ![nameNs isKindOfClass:[NSString class]] || [nameNs length] == 0) {
            nameNs = execNameNs;
        }
        QString name = stringFromNSString(nameNs);
        entries_[QStringLiteral("Name")] = name;
        NSString *iconFileNs = root[@"CFBundleIconFile"];
        icon_ = iconFileNs && [iconFileNs isKindOfClass:[NSString class]] && [iconFileNs length] > 0
                    ? stringFromNSString(iconFileNs)
                    : execName;
        startupWMClass_ = QString();
        categories_ = QStringList();
        mimeTypes_ = QStringList();
        noDisplay_ = false;
        valid_ = true;
    }
    return true;
}
