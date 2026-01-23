#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include "browserselector.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
int main(int argc, char *argv[]) {
#pragma clang diagnostic pop
#ifndef Q_OS_LINUX
    QApplication app(argc, argv);
    QMessageBox::critical(nullptr,
                          QObject::tr("Browser Selector"),
                          QObject::tr("This application only supports Linux."));
    return 1;
#else
    if (argc < 2) {
        qCritical("Usage: %s <url>", argv[0]);
        return 1;
    }
    QString urlToOpen = QString::fromUtf8(argv[1]);
    BrowserSelector selector(urlToOpen);
    return selector.exec();
#endif
}
