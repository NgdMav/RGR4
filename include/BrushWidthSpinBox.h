#ifndef BRUSHWIDTHSPINBOX_H
#define BRUSHWIDTHSPINBOX_H

#include <QSpinBox>
#include <QKeyEvent>
#include <QWidget>

class BrushWidthSpinBox : public QSpinBox {
    Q_OBJECT
public:
    using QSpinBox::QSpinBox;

    void setCanvasWidget(QWidget* canvas) {
        canvasWidget = canvas;
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->modifiers() == Qt::ControlModifier &&
            (event->key() == Qt::Key_Z || event->key() == Qt::Key_Y)) {
            event->ignore();
        } else {
            QSpinBox::keyPressEvent(event);
        }
    }

    void focusOutEvent(QFocusEvent *event) override {
        QSpinBox::focusOutEvent(event);
        if (canvasWidget) {
            canvasWidget->setFocus();
        }
    }

private:
    QWidget* canvasWidget = nullptr;
};

#endif
