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

    // ����������ʾ���ӳ�ʱ�䣨���룩
    void setDisplayDelay(int delay) {
        m_displayDelay = delay;
        if (m_timer->isActive()) {
            m_timer->setInterval(m_displayDelay);
        }
    }

    // ��ȡ��ǰ�ӳ�ʱ��
    int displayDelay() const {
        return m_displayDelay;
    }

    // �����ı����������ֻ�Ч��
    void setTextWithAnimation(const QString& text) {
        m_fullText = text;
        m_currentIndex = 0;
        m_animationComplete = false;
        QLabel::setText("");
        m_timer->start(m_displayDelay);
    }

    // ��дsetText�Ա��ּ�����
    void setText(const QString& text) {
        m_fullText = text;
        m_currentIndex = 0;
        m_animationComplete = false;
        QLabel::setText("");
        m_timer->start(m_displayDelay);
    }

    // ����������������ʾȫ���ı�
    void skipAnimation() {
        if (!m_animationComplete) {
            m_timer->stop();
            m_currentIndex = m_fullText.length();
            m_animationComplete = true;
            QLabel::setText(m_fullText);
            update();
        }
    }

    // ��鶯���Ƿ����
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

        // ��͸����ɫ���
        QPen pen(QColor(255, 255, 0, 180)); // ��͸����ɫ
        pen.setWidth(0.6);                    // ������ϸ���ɵ���
        painter.setPen(pen);
        painter.setBrush(Qt::white);        // �����ɫ
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