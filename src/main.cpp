#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include "browserchooser.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
int main(int argc, char *argv[]) {
#pragma clang diagnostic pop
    QSettings::setDefaultFormat(QSettings::IniFormat);
#ifndef Q_OS_LINUX
    QApplication app(argc, argv);
    QMessageBox::critical(nullptr,
                          QObject::tr("Browser Chooser"),
                          QObject::tr("This application only supports Linux."));
    return 1;
#else
    if (argc < 2) {
        qCritical("Usage: %s <url>", argv[0]);
        return 1;
    }
    auto urlToOpen = QString::fromUtf8(argv[1]);
    BrowserChooser chooser(urlToOpen);
    return chooser.exec();
#endif
}
