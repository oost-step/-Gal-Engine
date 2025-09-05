#pragma once
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

class OutlineLabel : public QLabel {
    Q_OBJECT
public:
    using QLabel::QLabel; // �̳й��캯��

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        QFont font = this->font();
        QString text = this->text();

        QPainterPath path;
        path.addText(0, fontMetrics().ascent(), font, text);

        // ��͸����ɫ���
        QPen pen(QColor(255, 255, 0, 180)); // ��͸����ɫ
        pen.setWidth(0.6);                    // ������ϸ���ɵ���
        painter.setPen(pen);
        painter.setBrush(Qt::white);        // �����ɫ
        painter.drawPath(path);
    }
};
