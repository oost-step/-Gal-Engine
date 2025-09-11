#pragma once
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

class OutlineLabel : public QLabel {
    Q_OBJECT
public:
    using QLabel::QLabel;

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        QFont font = this->font();
        QString text = this->text();

        QPainterPath path;
        path.addText(0, fontMetrics().ascent(), font, text);

        // QPen pen(QColor(255, 255, 0, 180));  // 半透明黄色边
        QPen pen(QColor(255, 255, 255, 180));
        pen.setWidthF(1.0);
        painter.setPen(pen);
        // painter.setBrush(Qt::white);  // 白色字体
        painter.setBrush(Qt::black);
        painter.drawPath(path);
    }
};
