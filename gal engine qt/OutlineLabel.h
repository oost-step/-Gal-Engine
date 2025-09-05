#pragma once
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

class OutlineLabel : public QLabel {
    Q_OBJECT
public:
    using QLabel::QLabel; // 继承构造函数

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        QFont font = this->font();
        QString text = this->text();

        QPainterPath path;
        path.addText(0, fontMetrics().ascent(), font, text);

        // 半透明黄色描边
        QPen pen(QColor(255, 255, 0, 180)); // 半透明黄色
        pen.setWidth(0.6);                    // 轮廓粗细（可调）
        painter.setPen(pen);
        painter.setBrush(Qt::white);        // 填充颜色
        painter.drawPath(path);
    }
};
