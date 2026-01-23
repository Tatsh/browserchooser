#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtGui/QIcon>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

#include "browserselector.h"
#include "selectorwidget.h"

SelectorWidget::SelectorWidget(BrowserSelector *selector, QWidget *parent)
    : DraggablePopup(parent), selector_(selector) {
    setupWindow();
}

QString SelectorWidget::getBaseDomain(const QString &domain) {
    // Extract base domain (e.g., google.com from www.google.com)
    auto parts = domain.split(QLatin1Char('.'));
    if (parts.size() <= 2) {
        return domain;
    }
    // Return last two parts (handles most cases like example.com)
    // For co.uk style domains this isn't perfect, but good enough
    return parts.mid(parts.size() - 2).join(QLatin1Char('.'));
}

void SelectorWidget::setupWindow() {
    // Create remember checkbox if we have a parsed domain
    const auto &parsedDomain = selector_->parsedDomain();
    if (parsedDomain.has_value()) {
        domain_ = *parsedDomain;
        baseDomain_ = getBaseDomain(domain_);
        rememberCheckBox_ = new QCheckBox(this);
        rememberCheckBox_->setText(tr("Do not ask again"));
        rememberCheckBox_->setChecked(true);
        connect(rememberCheckBox_,
                &QCheckBox::toggled,
                this,
                &SelectorWidget::onRememberCheckBoxToggled);
        // Create radio buttons for domain options
        radioContainer_ = new QWidget(this);
        auto *radioLayout = new QVBoxLayout(radioContainer_);
        radioLayout->setContentsMargins(20, 0, 0, 0);
        wildcardDomainRadio_ = new QRadioButton(
            tr("Open %1 and all subdomains with this browser").arg(baseDomain_), radioContainer_);
        exactDomainRadio_ =
            new QRadioButton(tr("Open only %1 with this browser").arg(domain_), radioContainer_);
        wildcardDomainRadio_->setChecked(true);
        radioLayout->addWidget(wildcardDomainRadio_);
        radioLayout->addWidget(exactDomainRadio_);
        radioContainer_->setVisible(true);
    }
    // Create layout with margins for draggable area
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(10, 10, 10, 10);
    auto *browserLayout = new QHBoxLayout();
    browserLayout->setSpacing(10);
    // Create browser entries (icon + label)
    const auto &browsers = selector_->availableBrowsers();
    for (int i = 0; i < browsers.size(); ++i) {
        QWidget *entry = createBrowserEntry(browsers[i], i);
        browserLayout->addWidget(entry);
    }
    outerLayout->addLayout(browserLayout);
    // Add checkbox and radio buttons if present
    if (rememberCheckBox_) {
        outerLayout->addWidget(rememberCheckBox_);
        outerLayout->addWidget(radioContainer_);
    }
    setLayout(outerLayout);
    // Focus first button
    if (firstButton_) {
        firstButton_->setFocus();
    }
    setWindowTitle(tr("Open URL with"));
}

void SelectorWidget::onRememberCheckBoxToggled(bool checked) {
    if (radioContainer_) {
        radioContainer_->setVisible(checked);
        adjustSize();
    }
}

void SelectorWidget::onButtonClicked(int browserIndex) {
    const auto &browsers = selector_->availableBrowsers();
    if (browserIndex < 0 || browserIndex >= browsers.size()) {
        return;
    }
    const DesktopEntry &browser = browsers[browserIndex];
    // Remember if checkbox is checked
    if (rememberCheckBox_ && rememberCheckBox_->isChecked()) {
        QString domainPattern;
        if (wildcardDomainRadio_ && wildcardDomainRadio_->isChecked()) {
            domainPattern = QStringLiteral("*.") + baseDomain_;
        } else {
            domainPattern = domain_;
        }
        selector_->remember(browser, domainPattern);
    }
    selector_->openBrowser(browser);
    close();
}

auto SelectorWidget::createBrowserEntry(const DesktopEntry &entry, int index) -> QWidget * {
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    // Create icon-only button with fixed size
    auto *button = new QToolButton(container);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setFixedSize(kIconSize + 16, kIconSize + 16);
    button->setIconSize(QSize(kIconSize, kIconSize));
    // Try to find and set the icon
    auto iconPath = findIconPath(entry.icon());
    if (!iconPath.isEmpty()) {
        button->setIcon(QIcon(iconPath));
    } else {
        // Try using the icon name directly (Qt might find it in theme)
        QIcon icon = QIcon::fromTheme(entry.icon());
        if (!icon.isNull()) {
            button->setIcon(icon);
        }
    }
    // Build tooltip with full command path
    auto command = entry.exec();
    const QString &url = selector_->urlToOpen();
    if (!url.isEmpty()) {
        if (command.contains(QStringLiteral("%U"))) {
            command.replace(QStringLiteral("%U"), url);
        } else if (command.contains(QStringLiteral("%u"))) {
            command.replace(QStringLiteral("%u"), url);
        }
    } else {
        command.remove(QStringLiteral("%U"));
        command.remove(QStringLiteral("%u"));
    }
    auto tooltip = command.simplified();
    auto parts = tooltip.split(QLatin1Char(' '));
    if (!parts.isEmpty()) {
        auto executable = parts.first();
        auto fullPath = QStandardPaths::findExecutable(executable);
        if (!fullPath.isEmpty()) {
            parts[0] = fullPath;
            tooltip = parts.join(QLatin1Char(' '));
        }
    }
    button->setToolTip(tooltip);
    // Create centered label with word wrap (up to 3 lines)
    auto *label = new QLabel(entry.name(), container);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    label->setWordWrap(true);
    label->setFixedWidth(kEntryWidth);
    label->setMaximumHeight(label->fontMetrics().lineSpacing() * 3);
    layout->addWidget(button, 0, Qt::AlignHCenter);
    layout->addWidget(label, 0, Qt::AlignHCenter);
    connect(button, &QToolButton::clicked, this, [this, index]() { onButtonClicked(index); });
    if (!firstButton_) {
        firstButton_ = button;
    }
    return container;
}

auto SelectorWidget::findIconPath(const QString &iconName) -> QString {
    // If it's already a path, return it
    if (iconName.startsWith(QLatin1Char('/')) && QFile::exists(iconName)) {
        return iconName;
    }
    // Common icon directories to search
    QStringList iconDirs = {QStringLiteral("/usr/share/icons/hicolor/48x48/apps"),
                            QStringLiteral("/usr/share/icons/hicolor/64x64/apps"),
                            QStringLiteral("/usr/share/icons/hicolor/128x128/apps"),
                            QStringLiteral("/usr/share/icons/hicolor/256x256/apps"),
                            QStringLiteral("/usr/share/icons/hicolor/scalable/apps"),
                            QStringLiteral("/usr/share/pixmaps")};
    // Add user icon directories
    auto homeDir = QDir::homePath();
    iconDirs.prepend(homeDir + QStringLiteral("/.local/share/icons/hicolor/48x48/apps"));
    iconDirs.prepend(homeDir + QStringLiteral("/.local/share/icons/hicolor/64x64/apps"));
    // Extensions to try
    QStringList extensions = {
        QStringLiteral(".png"), QStringLiteral(".svg"), QStringLiteral(".xpm")};
    for (const auto &dir : iconDirs) {
        for (const auto &ext : extensions) {
            auto path = dir + QLatin1Char('/') + iconName + ext;
            if (QFile::exists(path)) {
                return path;
            }
        }
    }
    return QString();
}
