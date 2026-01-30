#include <ranges>

#include <QtCore/QDir>
#include <QtCore/QMap>
#include <QtCore/QStandardPaths>
#include <QtGui/QCloseEvent>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

#include "backend.h"
#include "browserchooser.h"
#include "desktopentry.h"
#include "selectorwidget.h"

namespace {

QString formatBrowserInfoHtml(const QString &commandLine, const QString &desktopPath) {
    const auto nowrap = QStringLiteral("<span style='white-space: nowrap;'>");
    const auto endSpan = QStringLiteral("</span>");
    return QStringLiteral("<span style='font-weight: normal; font-size: 11px;'>"
                          "%1<h3>Command line</h3><code>%2</code>%3<br>"
                          "%1<h3>Desktop file</h3><code>%4</code>%3</span>")
        .arg(nowrap, commandLine.toHtmlEscaped(), endSpan, desktopPath.toHtmlEscaped());
}

} // anonymous namespace

SelectorWidget::SelectorWidget(BrowserChooser *chooser, QWidget *parent)
    : DraggablePopup(parent), chooser_(chooser) {
    setupWindow();
}

QString SelectorWidget::getBaseDomain(const QString &domain) {
    // Extract base domain (e.g., google.com from www.google.com).
    auto parts = domain.split(QLatin1Char('.'));
    if (parts.size() <= 2) {
        return domain;
    }
    // Return last two parts (handles most cases like example.com).
    // For co.uk style domains this isn't perfect, but good enough.
    return parts.mid(parts.size() - 2).join(QLatin1Char('.'));
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
        wildcardDomainRadio_ = new QRadioButton(
            tr("Open %1 and all subdomains with this browser").arg(baseDomain_), radioContainer_);
        exactDomainRadio_ =
            new QRadioButton(tr("Open only %1 with this browser").arg(domain_), radioContainer_);
        wildcardDomainRadio_->setChecked(true);
        radioLayout->addWidget(wildcardDomainRadio_);
        radioLayout->addWidget(exactDomainRadio_);
        radioContainer_->setVisible(rememberCheckBox_->isChecked());
    }
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(10, 10, 10, 10);
    const auto &browsers = chooser_->availableBrowsers();
    QMap<QString, QList<int>> byBrowser;
    QList<int> otherIndices;
    QList<int> guestIndices;
    for (auto i = 0; i < browsers.size(); ++i) {
        if (browsers[i].profileName() == QStringLiteral("Guest")) {
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

    const auto kSectionHeaderStyle =
        QStringLiteral("font-weight: bold; font-size: 13px; margin-top: 2px; margin-bottom: 8px;");
    const auto kSectionSpacing = 12;
    auto addSection = [this,
                       &browsers,
                       outerLayout,
                       kSectionHeaderStyle,
                       columnsPerRow,
                       &makePlaceholder](const QString &header,
                                         const QList<int> &indices,
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
        auto *label = new QLabel(header, this);
        label->setStyleSheet(kSectionHeaderStyle);
        outerLayout->addWidget(label);
        auto *grid = new QGridLayout();
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
        addSection(pair.first, pair.second, true, true, false);
    }
    addSection(tr("Other browsers"), otherIndices, false, false, true);
    if (!guestIndices.isEmpty()) {
        guestSectionWidget_ = new QWidget(this);
        auto *guestLayout = new QVBoxLayout(guestSectionWidget_);
        guestLayout->setContentsMargins(0, 0, 0, 0);
        auto *guestLabel = new QLabel(tr("Guest profiles"), guestSectionWidget_);
        guestLabel->setStyleSheet(kSectionHeaderStyle);
        guestLayout->addWidget(guestLabel);
        auto *guestGrid = new QGridLayout();
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
        guestSectionWidget_->setVisible(chooser_->showGuestProfiles());
        outerLayout->addWidget(guestSectionWidget_);
        outerLayout->addSpacing(6);
        auto *separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        outerLayout->addWidget(separator);
        showGuestCheckBox_ = new QCheckBox(this);
        showGuestCheckBox_->setText(tr("Show Guest profiles"));
        showGuestCheckBox_->setChecked(chooser_->showGuestProfiles());
        connect(showGuestCheckBox_,
                &QCheckBox::toggled,
                this,
                &SelectorWidget::onShowGuestCheckBoxToggled);
        outerLayout->addWidget(showGuestCheckBox_);
    } else {
        auto *separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        outerLayout->addWidget(separator);
    }
    if (rememberCheckBox_) {
        outerLayout->addWidget(rememberCheckBox_);
        outerLayout->addWidget(radioContainer_);
    }
    setLayout(outerLayout);
    // Focus first button.
    if (firstButton_) {
        firstButton_->setFocus();
    }
    setWindowTitle(tr("Open URL with"));
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

void SelectorWidget::onShowGuestCheckBoxToggled(bool checked) {
    chooser_->setShowGuestProfiles(checked);
    if (guestSectionWidget_) {
        guestSectionWidget_->setVisible(checked);
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
        if (wildcardDomainRadio_ && wildcardDomainRadio_->isChecked()) {
            domainPattern = QStringLiteral("*.") + baseDomain_;
        } else {
            domainPattern = domain_;
        }
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
    auto isGuest = option.profileName() == QStringLiteral("Guest");
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
    auto iconPath = findIconPath(entry.icon());
    if (!iconPath.isEmpty()) {
        button->setIcon(QIcon(iconPath));
    } else {
        auto icon = QIcon::fromTheme(entry.icon());
        if (!icon.isNull()) {
            button->setIcon(icon);
        }
    }
    const auto commandLine = getCommandLineForDisplay(option, chooser_->urlToOpen());
    const auto html = formatBrowserInfoHtml(commandLine, option.desktopPath());
    tooltipByIndex_[index] = html;
    QString labelText;
    if (browserNameOnly || option.profileName() == QStringLiteral("Guest")) {
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
    auto iconDirs = QStringList{QStringLiteral("/usr/share/icons/hicolor/48x48/apps"),
                                QStringLiteral("/usr/share/icons/hicolor/64x64/apps"),
                                QStringLiteral("/usr/share/icons/hicolor/128x128/apps"),
                                QStringLiteral("/usr/share/icons/hicolor/256x256/apps"),
                                QStringLiteral("/usr/share/icons/hicolor/scalable/apps"),
                                QStringLiteral("/usr/share/pixmaps")};
    // Add user icon directories.
    auto homeDir = QDir::homePath();
    iconDirs.prepend(homeDir + QStringLiteral("/.local/share/icons/hicolor/48x48/apps"));
    iconDirs.prepend(homeDir + QStringLiteral("/.local/share/icons/hicolor/64x64/apps"));
    // Extensions to try.
    auto extensions =
        QStringList{QStringLiteral(".png"), QStringLiteral(".svg"), QStringLiteral(".xpm")};
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
