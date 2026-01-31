/** @file */
#pragma once

#include <QtCore/QList>
#include <QtCore/QMap>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>

#include "browseroption.h"
#include "draggablepopup.h"

class QToolButton;
class BrowserChooser;

/** Widget for selecting a browser. */
class SelectorWidget : public DraggablePopup {
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param chooser The BrowserChooser managing this widget.
     * @param parent The parent widget.
     */
    explicit SelectorWidget(BrowserChooser *chooser, QWidget *parent = nullptr);

private Q_SLOTS:
    void onButtonClicked(int browserIndex);
    void onRememberCheckBoxToggled(bool checked);
    void onShowGuestCheckBoxToggled(bool checked);
    void onHideBrowsersWithoutProfilesCheckBoxToggled(bool checked);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupWindow();
    QWidget *createBrowserEntry(const BrowserOption &option,
                                int index,
                                bool inSection = false,
                                bool browserNameOnly = false);
    QString findIconPath(const QString &iconName);
    QString getBaseDomain(const QString &domain);

    static constexpr int kBrowsersPerRow = 5;
    static constexpr int kIconSize = 48;
    static constexpr int kEntryWidth = 100;

    BrowserChooser *chooser_;
    QLabel *urlLabel_ = nullptr;
    QCheckBox *rememberCheckBox_ = nullptr;
    QCheckBox *showGuestCheckBox_ = nullptr;
    QCheckBox *hideBrowsersWithoutProfilesCheckBox_ = nullptr;
    QWidget *guestSectionWidget_ = nullptr;
    QWidget *otherSectionWidget_ = nullptr;
    QRadioButton *exactDomainRadio_ = nullptr;
    QRadioButton *wildcardDomainRadio_ = nullptr;
    QWidget *radioContainer_ = nullptr;
    QString domain_;
    QString baseDomain_;
    QString urlToOpen_;
    QToolButton *firstButton_ = nullptr;
    QMap<int, QString> tooltipByIndex_;
};
