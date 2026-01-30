#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

#include "draggablepopup.h"

DraggablePopup::DraggablePopup(QWidget *parent) : QWidget(parent) {
    // Frameless, stays on top.
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAutoFillBackground(true);
}

void DraggablePopup::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    // Install event filter on children for drag detection.
    installEventFilterOnChildren(this);
}

void DraggablePopup::installEventFilterOnChildren(QObject *parent) {
    for (QObject *child : parent->children()) {
        if (auto *widget = qobject_cast<QWidget *>(child)) {
            widget->installEventFilter(this);
            installEventFilterOnChildren(child);
        }
    }
}

void DraggablePopup::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        QApplication::quit();
        return;
    }
    QWidget::keyPressEvent(event);
}

void DraggablePopup::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (startDrag()) {
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void DraggablePopup::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
    }
    QWidget::mouseReleaseEvent(event);
}

bool DraggablePopup::startDrag() {
    if (QWindow *win = windowHandle()) {
        dragging_ = true;
        // Use system move for Wayland compatibility.
        return win->startSystemMove();
    }
    return false;
}

bool DraggablePopup::eventFilter(QObject *watched, QEvent *event) {
    // Forward mouse press from non-interactive children to enable dragging.
    if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton && !dragging_) {
            // Allow dragging from labels and frames (but not buttons).
            if (qobject_cast<QLabel *>(watched) || qobject_cast<QFrame *>(watched)) {
                startDrag();
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton && dragging_) {
            dragging_ = false;
        }
    }
    return false; // Don't consume events.
}
