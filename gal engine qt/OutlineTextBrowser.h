#pragma once
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QMouseEvent>
#include <QTextLayout>

class OutlineTextBrowser : public QLabel {
    Q_OBJECT
public:
    explicit OutlineTextBrowser(QWidget* parent = nullptr)
        : QLabel(parent), m_displayDelay(50), m_currentIndex(0), m_animationComplete(false) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &OutlineTextBrowser::updateText);

        setWordWrap(true);
    }

    // ÉèÖÃÖð×ÖÏÔÊ¾µÄÑÓ³ÙÊ±¼ä£¨ºÁÃë£©
    void setDisplayDelay(int delay) {
        m_displayDelay = delay;
        if (m_timer->isActive()) {
            m_timer->setInterval(m_displayDelay);
        }
    }

    int displayDelay() const { return m_displayDelay; }

    void setTextWithAnimation(const QString& text) {
        m_fullText = text;
        m_currentIndex = 0;
        m_animationComplete = false;
        QLabel::setText("");
        m_timer->start(m_displayDelay);
        update();
    }

    void setText(const QString& text) {
        m_fullText = text;
        m_currentIndex = 0;
        m_animationComplete = false;
        QLabel::setText("");
        m_timer->start(m_displayDelay);
        update();
    }

    void skipAnimation() {
        if (!m_animationComplete) {
            m_timer->stop();
            m_currentIndex = m_fullText.length();
            m_animationComplete = true;
            update();
        }
    }

    bool isAnimationComplete() const { return m_animationComplete; }

signals:
    void clicked();
    void animationCompleteClicked();

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        QFont font = this->font();
        QFontMetrics fm(font);

        QString displayText = m_animationComplete ? m_fullText : m_fullText.left(m_currentIndex);

        QTextLayout textLayout(displayText, font);
        textLayout.beginLayout();
        QVector<QTextLine> lines;
        while (true) {
            QTextLine line = textLayout.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(width());
            lines.append(line);
        }
        textLayout.endLayout();

        QPainterPath path;
        for (int i = 0; i < lines.size(); ++i) {
            QTextLine line = lines.at(i);
            qreal y = fm.ascent() + i * fm.lineSpacing();
            QString lineText = displayText.mid(line.textStart(), line.textLength());
            path.addText(0, y, font, lineText);
        }

        QPen pen(QColor(255, 255, 0, 180));
        pen.setWidthF(1.2);
        painter.setPen(pen);
        painter.setBrush(Qt::white);
        painter.drawPath(path);
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            if (!m_animationComplete) {
                skipAnimation();
            }
            else {
                emit animationCompleteClicked();
            }
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
