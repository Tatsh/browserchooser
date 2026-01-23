#pragma once

#include <QtCore/QStringList>

#include "desktopentry.h"

/**
 * Launch the given browser with the specified URLs.
 * If no URLs are provided, the browser is launched without any.
 * @param entry The DesktopEntry of the browser to launch.
 * @param urls A list of URLs to open in the browser.
 */
void launchBrowser(const DesktopEntry &entry, const QStringList &urls = QStringList());
