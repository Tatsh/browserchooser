/** @file */
#pragma once

#include <QtWidgets/QWidget>

/**
 * A frameless, draggable popup widget with the following features:
 * - No titlebar (frameless).
 * - Escape key quits the application.
 * - Dragging empty areas of the window moves it.
 * - Stays on top of other windows.
 */
class DraggablePopup : public QWidget {
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param parent The parent widget.
     */
    explicit DraggablePopup(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void installEventFilterOnChildren(QObject *parent);
    bool startDrag();

    bool dragging_ = false;
};
