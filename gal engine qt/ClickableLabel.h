#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(int index = -1, QWidget* parent = nullptr)
        : QLabel(parent), m_index(index) {
    }

signals:
    void clicked(int index);

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked(m_index);
        }
        QLabel::mousePressEvent(event);
    }

private:
    int m_index;
};

#endif // CLICKABLELABEL_H
