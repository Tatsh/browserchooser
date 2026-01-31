#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QMap>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QFontMetrics>
#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>

#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

#include "backend.h"
#include "basedomain.h"
#include "browserchooser.h"
#include "browserinfoformat.h"
#include "desktopentry.h"
#include "selectorwidget.h"
#include "stringconstants.h"

namespace {

static const auto kThemeApplicationsInternet = QStringLiteral("applications-internet");
static const auto kThemeWebBrowser = QStringLiteral("web-browser");
static const auto kThemeUserIdentity = QStringLiteral("user-identity");
static const auto kThemeUser = QStringLiteral("user");
static const auto kSectionHeaderStyle =
    QStringLiteral("font-weight: bold; font-size: 13px; margin-top: 2px; margin-bottom: 8px;");
static const auto kHelpUrl = QStringLiteral(BROWSERCHOOSER_HELP_URL);

/** Returns an icon with the image at @a path masked to a circle of @a size pixels. */
QIcon iconFromPathMaskedAsCircle(const QString &path, int size) {
    QPixmap source(path);
    if (source.isNull()) {
        return QIcon();
    }
    QPixmap out(size, size);
    out.fill(Qt::transparent);
    QPainter painter(&out);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath clipPath;
    clipPath.addEllipse(0, 0, size, size);
    painter.setClipPath(clipPath);
    painter.drawPixmap(
        0,
        0,
        size,
        size,
        source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    painter.end();
    return QIcon(out);
}

} // anonymous namespace

SelectorWidget::SelectorWidget(BrowserChooser *chooser, QWidget *parent)
    : DraggablePopup(parent), chooser_(chooser) {
    setupWindow();
}

QString SelectorWidget::getBaseDomain(const QString &domain) {
    return ::getBaseDomain(domain);
}

void SelectorWidget::setupWindow() {
    // Create remember checkbox if we have a parsed domain.
    const auto &parsedDomain = chooser_->parsedDomain();
    if (parsedDomain.has_value()) {
        domain_ = *parsedDomain;
        baseDomain_ = getBaseDomain(domain_);
        rememberCheckBox_ = new QCheckBox(this);
        rememberCheckBox_->setText(tr("Do not ask again"));
        rememberCheckBox_->setChecked(chooser_->rememberChoiceChecked());
        connect(rememberCheckBox_,
                &QCheckBox::toggled,
                this,
                &SelectorWidget::onRememberCheckBoxToggled);
        // Create radio buttons for domain options.
        radioContainer_ = new QWidget(this);
        auto *radioLayout = new QVBoxLayout(radioContainer_);
        radioLayout->setContentsMargins(20, 0, 0, 0);
        exactDomainRadio_ =
            new QRadioButton(tr("Open %1 with this browser").arg(domain_), radioContainer_);
        wildcardDomainRadio_ = new QRadioButton(
            tr("Open %1 and all subdomains with this browser").arg(baseDomain_), radioContainer_);
        const bool useWildcard = chooser_->rememberDomainWildcard();
        wildcardDomainRadio_->setChecked(useWildcard);
        exactDomainRadio_->setChecked(!useWildcard);
        radioLayout->addWidget(exactDomainRadio_);
        radioLayout->addWidget(wildcardDomainRadio_);
        radioContainer_->setVisible(rememberCheckBox_->isChecked());
    }
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(10, 10, 10, 10);
    urlToOpen_ = chooser_->urlToOpen();
    if (!urlToOpen_.isEmpty()) {
        urlLabel_ = new QLabel(this);
        urlLabel_->setMaximumWidth(500);
        urlLabel_->setToolTip(urlToOpen_);
        urlLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        urlLabel_->setStyleSheet(
            QStringLiteral("color: palette(placeholder-text); font-size: 11px;"));
        urlLabel_->setWordWrap(false);
        const auto w = urlLabel_->maximumWidth();
        const auto elided = urlLabel_->fontMetrics().elidedText(urlToOpen_, Qt::ElideMiddle, w);
        urlLabel_->setText(tr("Opening %1").arg(elided));
        outerLayout->addWidget(urlLabel_);
        outerLayout->addSpacing(6);
    }
    const auto &browsers = chooser_->availableBrowsers();
    QMap<QString, QList<int>> byBrowser;
    QList<int> otherIndices;
    QList<int> guestIndices;
    for (auto i = 0; i < browsers.size(); ++i) {
        if (browsers[i].profileName() == kGuest) {
            guestIndices.append(i);
            continue;
        }
        byBrowser[browsers[i].desktopPath()].append(i);
    }
    for (auto it = byBrowser.begin(); it != byBrowser.end(); ++it) {
        if (it->size() == 1) {
            otherIndices.append(it->first());
        }
    }
    auto maxSectionSize = 0;
    for (auto it = byBrowser.begin(); it != byBrowser.end(); ++it) {
        maxSectionSize = std::max(maxSectionSize, static_cast<int>(it->size()));
    }
    maxSectionSize = std::max(maxSectionSize, static_cast<int>(otherIndices.size()));
    maxSectionSize = std::max(maxSectionSize, static_cast<int>(guestIndices.size()));
    const auto columnsPerRow = std::min(kBrowsersPerRow, std::max(1, maxSectionSize));

    auto makePlaceholder = [this]() {
        auto *w = new QWidget(this);
        w->setFixedWidth(kEntryWidth);
        w->setFixedHeight(1);
        w->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        return w;
    };

    const auto kSectionHeaderIconSize = 16;
    const auto kSectionHeaderContentOffset = kSectionHeaderIconSize + 6;
    const auto kSectionSpacing = 12;
    const auto kSectionHeaderIconNudge = 5;
    auto createSectionHeader =
        [kSectionHeaderIconSize, kSectionHeaderIconNudge](
            const QString &text, const QIcon &icon, QWidget *parent) -> QWidget * {
        auto *w = new QWidget(parent);
        auto *layout = new QHBoxLayout(w);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(6);
        layout->setAlignment(Qt::AlignVCenter);
        if (!icon.isNull()) {
            auto *iconContainer = new QWidget(w);
            iconContainer->setFixedSize(kSectionHeaderIconSize,
                                        kSectionHeaderIconSize + kSectionHeaderIconNudge);
            auto *iconLayout = new QVBoxLayout(iconContainer);
            iconLayout->setContentsMargins(0, 0, 0, 0);
            iconLayout->setSpacing(0);
            auto *iconLabel = new QLabel(iconContainer);
            iconLabel->setPixmap(icon.pixmap(kSectionHeaderIconSize, kSectionHeaderIconSize));
            iconLabel->setFixedSize(kSectionHeaderIconSize, kSectionHeaderIconSize);
            iconLayout->addWidget(iconLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
            layout->addWidget(iconContainer, 0, Qt::AlignVCenter);
        }
        auto *label = new QLabel(text, w);
        label->setStyleSheet(kSectionHeaderStyle);
        layout->addWidget(label, 1, Qt::AlignVCenter);
        return w;
    };
    auto addSection = [this,
                       &browsers,
                       outerLayout,
                       createSectionHeader,
                       kSectionHeaderContentOffset,
                       columnsPerRow,
                       &makePlaceholder](const QString &header,
                                         const QList<int> &indices,
                                         const QIcon &headerIcon,
                                         bool inSection = false,
                                         bool sortDefaultFirst = false,
                                         bool browserNameOnly = false) {
        if (indices.isEmpty()) {
            return;
        }
        QList<int> sorted = indices;
        if (sortDefaultFirst) {
            std::ranges::sort(sorted, [&browsers](int a, int b) {
                auto aDefault = browsers[a].profileName().isEmpty();
                auto bDefault = browsers[b].profileName().isEmpty();
                if (aDefault != bDefault) {
                    return aDefault;
                }
                return a < b;
            });
        }
        outerLayout->addWidget(createSectionHeader(header, headerIcon, this));
        auto *grid = new QGridLayout();
        grid->setContentsMargins(kSectionHeaderContentOffset, 0, 0, 0);
        grid->setSpacing(10);
        for (auto j = 0; j < sorted.size(); ++j) {
            auto browserIndex = sorted[j];
            QWidget *entry = createBrowserEntry(
                browsers[browserIndex], browserIndex, inSection, browserNameOnly);
            grid->addWidget(
                entry, j / columnsPerRow, j % columnsPerRow, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        }
        const auto lastRow = static_cast<int>(sorted.size() - 1) / columnsPerRow;
        const auto lastRowCount = static_cast<int>(sorted.size()) - lastRow * columnsPerRow;
        for (auto col = lastRowCount; col < columnsPerRow; ++col) {
            grid->addWidget(makePlaceholder(), lastRow, col, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        }
        for (auto c = 0; c < columnsPerRow; ++c) {
            grid->setColumnStretch(c, 0);
        }
        outerLayout->addLayout(grid);
        outerLayout->addSpacing(kSectionSpacing);
    };
    QList<QPair<QString, QList<int>>> multiProfile;
    for (auto it = byBrowser.begin(); it != byBrowser.end(); ++it) {
        if (it->size() > 1) {
            multiProfile.append({browsers[it->first()].entry().name(), *it});
        }
    }
    std::ranges::sort(multiProfile, [](const auto &a, const auto &b) {
        return a.first.compare(b.first, Qt::CaseInsensitive) < 0;
    });
    for (const auto &pair : multiProfile) {
        QIcon browserIcon;
        const auto entry = browsers[pair.second.first()].entry();
        const auto iconPath = findIconPath(entry.icon());
        if (!iconPath.isEmpty()) {
            browserIcon = QIcon(iconPath);
        } else {
            browserIcon = QIcon::fromTheme(entry.icon());
        }
        addSection(pair.first, pair.second, browserIcon, true, true, false);
    }
    QIcon webIcon = QIcon::fromTheme(kThemeApplicationsInternet);
    if (webIcon.isNull()) {
        webIcon = QIcon::fromTheme(kThemeWebBrowser);
    }
    if (!otherIndices.isEmpty()) {
        otherSectionWidget_ = new QWidget(this);
        auto *otherLayout = new QVBoxLayout(otherSectionWidget_);
        otherLayout->setContentsMargins(0, 0, 0, 0);
        otherLayout->addWidget(
            createSectionHeader(tr("Other browsers"), webIcon, otherSectionWidget_));
        auto *otherGrid = new QGridLayout();
        otherGrid->setContentsMargins(kSectionHeaderContentOffset, 0, 0, 0);
        otherGrid->setSpacing(10);
        for (auto j = 0; j < otherIndices.size(); ++j) {
            auto browserIndex = otherIndices[j];
            QWidget *entry = createBrowserEntry(browsers[browserIndex], browserIndex, false, true);
            otherGrid->addWidget(
                entry, j / columnsPerRow, j % columnsPerRow, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        }
        const auto otherLastRow = static_cast<int>(otherIndices.size() - 1) / columnsPerRow;
        const auto otherLastRowCount =
            static_cast<int>(otherIndices.size()) - otherLastRow * columnsPerRow;
        for (auto col = otherLastRowCount; col < columnsPerRow; ++col) {
            otherGrid->addWidget(
                makePlaceholder(), otherLastRow, col, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        }
        for (auto c = 0; c < columnsPerRow; ++c) {
            otherGrid->setColumnStretch(c, 0);
        }
        otherLayout->addLayout(otherGrid);
        otherLayout->addSpacing(kSectionSpacing);
        outerLayout->addWidget(otherSectionWidget_);
        otherSectionWidget_->setVisible(!chooser_->hideBrowsersWithoutProfiles());
    }
    if (!guestIndices.isEmpty()) {
        guestSectionWidget_ = new QWidget(this);
        auto *guestLayout = new QVBoxLayout(guestSectionWidget_);
        guestLayout->setContentsMargins(0, 0, 0, 0);
        QIcon userIcon = QIcon::fromTheme(kThemeUserIdentity);
        if (userIcon.isNull()) {
            userIcon = QIcon::fromTheme(kThemeUser);
        }
        guestLayout->addWidget(
            createSectionHeader(tr("Guest profiles"), userIcon, guestSectionWidget_));
        auto *guestGrid = new QGridLayout();
        guestGrid->setContentsMargins(kSectionHeaderContentOffset, 0, 0, 0);
        guestGrid->setSpacing(10);
        for (auto j = 0; j < guestIndices.size(); ++j) {
            auto browserIndex = guestIndices[j];
            QWidget *entry = createBrowserEntry(browsers[browserIndex], browserIndex, false);
            guestGrid->addWidget(
                entry, j / columnsPerRow, j % columnsPerRow, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        }
        const auto guestLastRow = static_cast<int>(guestIndices.size() - 1) / columnsPerRow;
        const auto guestLastRowCount =
            static_cast<int>(guestIndices.size()) - guestLastRow * columnsPerRow;
        for (auto col = guestLastRowCount; col < columnsPerRow; ++col) {
            guestGrid->addWidget(
                makePlaceholder(), guestLastRow, col, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        }
        for (auto c = 0; c < columnsPerRow; ++c) {
            guestGrid->setColumnStretch(c, 0);
        }
        guestLayout->addLayout(guestGrid);
        guestSectionWidget_->setVisible(!chooser_->hideGuestProfiles());
        outerLayout->addWidget(guestSectionWidget_);
        outerLayout->addSpacing(6);
    }
    auto *separatorBeforeRemember = new QFrame(this);
    separatorBeforeRemember->setFrameShape(QFrame::HLine);
    separatorBeforeRemember->setFrameShadow(QFrame::Sunken);
    outerLayout->addWidget(separatorBeforeRemember);
    if (rememberCheckBox_) {
        outerLayout->addWidget(rememberCheckBox_);
        outerLayout->addWidget(radioContainer_);
    }
    if (!guestIndices.isEmpty()) {
        showGuestCheckBox_ = new QCheckBox(this);
        showGuestCheckBox_->setText(tr("Hide Guest profiles"));
        showGuestCheckBox_->setChecked(chooser_->hideGuestProfiles());
        connect(showGuestCheckBox_,
                &QCheckBox::toggled,
                this,
                &SelectorWidget::onShowGuestCheckBoxToggled);
        outerLayout->addWidget(showGuestCheckBox_);
    }
    hideBrowsersWithoutProfilesCheckBox_ = new QCheckBox(this);
    hideBrowsersWithoutProfilesCheckBox_->setText(tr("Hide browsers without profiles"));
    hideBrowsersWithoutProfilesCheckBox_->setChecked(chooser_->hideBrowsersWithoutProfiles());
    connect(hideBrowsersWithoutProfilesCheckBox_,
            &QCheckBox::toggled,
            this,
            &SelectorWidget::onHideBrowsersWithoutProfilesCheckBoxToggled);
    outerLayout->addWidget(hideBrowsersWithoutProfilesCheckBox_);
    setLayout(outerLayout);
    // Focus first button.
    if (firstButton_) {
        firstButton_->setFocus();
    }
    setWindowTitle(tr("Open URL with"));
    QIcon appIcon = QIcon::fromTheme(kThemeApplicationsInternet);
    if (appIcon.isNull()) {
        appIcon = QIcon::fromTheme(kThemeWebBrowser);
    }
    if (!appIcon.isNull()) {
        setWindowIcon(appIcon);
    }
}

void SelectorWidget::onRememberCheckBoxToggled(bool checked) {
    chooser_->setRememberChoiceChecked(checked);
    if (radioContainer_) {
        radioContainer_->setVisible(checked);
        adjustSize();
    }
}

void SelectorWidget::closeEvent(QCloseEvent *event) {
    if (rememberCheckBox_) {
        chooser_->setRememberChoiceChecked(rememberCheckBox_->isChecked());
    }
    DraggablePopup::closeEvent(event);
}

void SelectorWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_F1) {
        QDesktopServices::openUrl(QUrl(kHelpUrl));
        event->accept();
        return;
    }
    DraggablePopup::keyPressEvent(event);
}

void SelectorWidget::onShowGuestCheckBoxToggled(bool checked) {
    chooser_->setHideGuestProfiles(checked);
    if (guestSectionWidget_) {
        guestSectionWidget_->setVisible(!checked);
    }
    adjustSize();
}

void SelectorWidget::onHideBrowsersWithoutProfilesCheckBoxToggled(bool checked) {
    chooser_->setHideBrowsersWithoutProfiles(checked);
    if (otherSectionWidget_) {
        otherSectionWidget_->setVisible(!checked);
    }
    adjustSize();
}

void SelectorWidget::onButtonClicked(int browserIndex) {
    const auto &browsers = chooser_->availableBrowsers();
    if (browserIndex < 0 || browserIndex >= browsers.size()) {
        return;
    }
    if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) != 0) {
        auto it = tooltipByIndex_.constFind(browserIndex);
        if (it != tooltipByIndex_.constEnd()) {
            QMessageBox msg(this);
            msg.setWindowTitle(tr("Browser information"));
            msg.setTextFormat(Qt::RichText);
            msg.setText(*it);
            msg.exec();
        }
        return;
    }
    const BrowserOption &option = browsers[browserIndex];
    if (rememberCheckBox_ && rememberCheckBox_->isChecked()) {
        QString domainPattern;
        const bool useWildcard = wildcardDomainRadio_ && wildcardDomainRadio_->isChecked();
        if (useWildcard) {
            static const auto kFmtWildcardDomain = QStringLiteral("*.%1");
            domainPattern = kFmtWildcardDomain.arg(baseDomain_);
        } else {
            domainPattern = domain_;
        }
        chooser_->setRememberDomainWildcard(useWildcard);
        chooser_->remember(option, domainPattern);
    }
    chooser_->openBrowser(option);
    close();
}

auto SelectorWidget::createBrowserEntry(const BrowserOption &option,
                                        int index,
                                        bool inSection,
                                        bool browserNameOnly) -> QWidget * {
    auto entry = option.entry();
    auto *container = new QWidget(this);
    auto isGuest = option.profileName() == kGuest;
    container->setProperty("isGuest", isGuest);
    container->setVisible(true);
    container->setMaximumWidth(kEntryWidth);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    auto *button = new QToolButton(container);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setFixedSize(kIconSize + 16, kIconSize + 16);
    button->setIconSize(QSize(kIconSize, kIconSize));
    const auto pathLower = option.desktopPath().toLower();
    static const auto kChrome = QStringLiteral("chrome");
    static const auto kChromium = QStringLiteral("chromium");
    const bool isChromeProfile = option.profileName() != kGuest &&
                                 (pathLower.contains(kChrome) || pathLower.contains(kChromium));
    QIcon iconToUse;
    if (isChromeProfile) {
        const auto picturePath =
            getChromeProfilePicturePath(option.desktopPath(), option.profileName());
        if (!picturePath.isEmpty()) {
            iconToUse = iconFromPathMaskedAsCircle(picturePath, kIconSize);
        }
        if (iconToUse.isNull()) {
            iconToUse = QIcon::fromTheme(kThemeUserIdentity);
        }
        if (iconToUse.isNull()) {
            iconToUse = QIcon::fromTheme(kThemeUser);
        }
    }
    if (!iconToUse.isNull()) {
        button->setIcon(iconToUse);
    } else {
        auto iconPath = findIconPath(entry.icon());
        if (!iconPath.isEmpty()) {
            button->setIcon(QIcon(iconPath));
        } else {
            auto icon = QIcon::fromTheme(entry.icon());
            if (!icon.isNull()) {
                button->setIcon(icon);
            }
        }
    }
    const auto commandLine = getCommandLineForDisplay(option, chooser_->urlToOpen());
    const auto html = ::formatBrowserInfoHtml(commandLine, option.desktopPath());
    tooltipByIndex_[index] = html;
    QString labelText;
    if (browserNameOnly || option.profileName() == kGuest) {
        labelText = option.entry().name();
    } else if (inSection) {
        labelText = option.profileLabel();
    } else {
        labelText = option.displayName();
    }
    auto *label = new QLabel(labelText, container);
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
    // If it's already a path, return it.
    if (iconName.startsWith(QLatin1Char('/')) && QFile::exists(iconName)) {
        return iconName;
    }
    // Common icon directories to search.
    static const auto kIconDir48 = QStringLiteral("/usr/share/icons/hicolor/48x48/apps");
    static const auto kIconDir64 = QStringLiteral("/usr/share/icons/hicolor/64x64/apps");
    static const auto kIconDir128 = QStringLiteral("/usr/share/icons/hicolor/128x128/apps");
    static const auto kIconDir256 = QStringLiteral("/usr/share/icons/hicolor/256x256/apps");
    static const auto kIconDirScalable = QStringLiteral("/usr/share/icons/hicolor/scalable/apps");
    static const auto kPixmaps = QStringLiteral("/usr/share/pixmaps");
    auto iconDirs =
        QStringList{kIconDir48, kIconDir64, kIconDir128, kIconDir256, kIconDirScalable, kPixmaps};
    // Add user icon directories.
    auto homeDir = QDir::homePath();
    static const auto kFmtIconDir48 = QStringLiteral("%1/.local/share/icons/hicolor/48x48/apps");
    static const auto kFmtIconDir64 = QStringLiteral("%1/.local/share/icons/hicolor/64x64/apps");
    iconDirs.prepend(kFmtIconDir48.arg(homeDir));
    iconDirs.prepend(kFmtIconDir64.arg(homeDir));
    // Extensions to try.
    static const auto kExtPng = QStringLiteral(".png");
    static const auto kExtSvg = QStringLiteral(".svg");
    static const auto kExtXpm = QStringLiteral(".xpm");
    auto extensions = QStringList{kExtPng, kExtSvg, kExtXpm};
    for (const auto &dir : iconDirs) {
        for (const auto &ext : extensions) {
            static const auto kFmtIconPath = QStringLiteral("%1/%2%3");
            auto path = kFmtIconPath.arg(dir, iconName, ext);
            if (QFile::exists(path)) {
                return path;
            }
        }
    }
    return QString();
}
