#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include "browserchooser.h"

int main(int argc, char *argv[]) {
    QSettings::setDefaultFormat(QSettings::IniFormat);
    if (argc < 2) {
        qCritical("Usage: %s <url>", argv[0]);
        return 1;
    }
    auto urlToOpen = QString::fromUtf8(argv[1]);
    BrowserChooser chooser(urlToOpen);
    return chooser.exec();
}
