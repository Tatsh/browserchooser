#pragma once

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>

#include "desktopentry.h"
#include "draggablepopup.h"

class QToolButton;
class BrowserSelector;

/** Widget for selecting a browser. */
class SelectorWidget : public DraggablePopup {
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param selector The BrowserSelector managing this widget.
     * @param parent The parent widget.
     */
    explicit SelectorWidget(BrowserSelector *selector, QWidget *parent = nullptr);

private Q_SLOTS:
    void onButtonClicked(int browserIndex);
    void onRememberCheckBoxToggled(bool checked);

private:
    void setupWindow();
    QWidget *createBrowserEntry(const DesktopEntry &entry, int index);
    QString findIconPath(const QString &iconName);
    QString getBaseDomain(const QString &domain);

    static constexpr int kIconSize = 48;
    static constexpr int kEntryWidth = 100;

    BrowserSelector *selector_;
    QCheckBox *rememberCheckBox_ = nullptr;
    QRadioButton *exactDomainRadio_ = nullptr;
    QRadioButton *wildcardDomainRadio_ = nullptr;
    QWidget *radioContainer_ = nullptr;
    QString domain_;
    QString baseDomain_;
    QToolButton *firstButton_ = nullptr;
};
