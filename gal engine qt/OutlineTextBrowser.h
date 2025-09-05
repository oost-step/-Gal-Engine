#pragma once
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QMouseEvent>

class OutlineTextBrowser : public QLabel {
    Q_OBJECT
public:
    explicit OutlineTextBrowser(QWidget* parent = nullptr)
        : QLabel(parent), m_displayDelay(50), m_currentIndex(0), m_animationComplete(false) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &OutlineTextBrowser::updateText);
    }

    // 设置逐字显示的延迟时间（毫秒）
    void setDisplayDelay(int delay) {
        m_displayDelay = delay;
        if (m_timer->isActive()) {
            m_timer->setInterval(m_displayDelay);
        }
    }

    // 获取当前延迟时间
    int displayDelay() const {
        return m_displayDelay;
    }

    // 设置文本并启动打字机效果
    void setTextWithAnimation(const QString& text) {
        m_fullText = text;
        m_currentIndex = 0;
        m_animationComplete = false;
        QLabel::setText("");
        m_timer->start(m_displayDelay);
    }

    // 重写setText以保持兼容性
    void setText(const QString& text) {
        m_fullText = text;
        m_currentIndex = 0;
        m_animationComplete = false;
        QLabel::setText("");
        m_timer->start(m_displayDelay);
    }

    // 跳过动画，立即显示全部文本
    void skipAnimation() {
        if (!m_animationComplete) {
            m_timer->stop();
            m_currentIndex = m_fullText.length();
            m_animationComplete = true;
            QLabel::setText(m_fullText);
            update();
        }
    }

    // 检查动画是否完成
    bool isAnimationComplete() const {
        return m_animationComplete;
    }

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        QFont font = this->font();
        QString displayText = m_animationComplete ? m_fullText : m_fullText.left(m_currentIndex);

        QPainterPath path;
        path.addText(0, fontMetrics().ascent(), font, displayText);

        // 半透明黄色描边
        QPen pen(QColor(255, 255, 0, 180)); // 半透明黄色
        pen.setWidth(0.6);                    // 轮廓粗细（可调）
        painter.setPen(pen);
        painter.setBrush(Qt::white);        // 填充颜色
        painter.drawPath(path);
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            if (!m_animationComplete) skipAnimation();
            else emit clicked();
        }
        QLabel::mousePressEvent(event);
    }
private slots:
    void updateText() {
        if (m_currentIndex < m_fullText.length()) {
            m_currentIndex++;
            update();
        }
        else {
            m_timer->stop();
            m_animationComplete = true;
        }
    }

private:
    QTimer* m_timer;
    int m_displayDelay;
    int m_currentIndex;
    bool m_animationComplete;
    QString m_fullText;
};