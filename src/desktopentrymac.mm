#include <QtCore/QFile>
#include <QtCore/QString>

#import <Foundation/Foundation.h>

#include "desktopentry.h"
#include "stringconstants.h"

static const auto kFmtPlistPath = QStringLiteral("%1/Contents/Info.plist");
static const auto kFmtMacOSPath = QStringLiteral("%1/Contents/MacOS/%2");

static NSDictionary *loadPlist(const QString &plistPath) {
    const QByteArray pathUtf8 = plistPath.toUtf8();
    NSString *path = [NSString stringWithUTF8String:pathUtf8.constData()];
    return [NSDictionary dictionaryWithContentsOfFile:path];
}

bool DesktopEntry::parseAppBundle(const QString &bundlePath) {
    valid_ = false;
    filename_ = bundlePath;
    entries_.clear();
    const auto plistPath = kFmtPlistPath.arg(bundlePath);
    if (!QFile::exists(plistPath)) {
        return false;
    }
    @autoreleasepool {
        NSDictionary *root = loadPlist(plistPath);
        if (!root) {
            return false;
        }
        NSString *execNameNs = root[@"CFBundleExecutable"];
        if (!execNameNs || ![execNameNs isKindOfClass:NSString.class] || execNameNs.length == 0) {
            return false;
        }
        auto execName = QString::fromNSString(execNameNs);
        exec_ = kFmtMacOSPath.arg(bundlePath, execName);
        if (!QFile::exists(exec_)) {
            return false;
        }
        NSString *nameNs = root[@"CFBundleDisplayName"];
        if (!nameNs || ![nameNs isKindOfClass:NSString.class] || nameNs.length == 0) {
            nameNs = root[@"CFBundleName"];
        }
        if (!nameNs || ![nameNs isKindOfClass:NSString.class] || nameNs.length == 0) {
            nameNs = execNameNs;
        }
        QString name = QString::fromNSString(nameNs);
        entries_[kName] = name;
        NSString *iconFileNs = root[@"CFBundleIconFile"];
        icon_ = iconFileNs && [iconFileNs isKindOfClass:NSString.class] && iconFileNs.length > 0 ?
                    QString::fromNSString(iconFileNs) :
                    execName;
        startupWMClass_ = QString();
        categories_ = QStringList();
        mimeTypes_ = QStringList();
        noDisplay_ = false;
        valid_ = true;
    }
    return true;
}
