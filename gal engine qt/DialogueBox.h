#pragma once
#include <QFrame>
#include <QWidget>
#include "OutlineLabel.h"
#include "OutlineTextBrowser.h"

class DialogueBox : public QFrame {
    Q_OBJECT
public:
    explicit DialogueBox(QWidget* parent = nullptr);
    void setSpeaker(const QString& name);
    void setText(const QString& t);

    bool isTyping() const { return m_text->isAnimationComplete(); }
    void skipTyping() { m_text->skipAnimation(); }

    OutlineTextBrowser* textWidget() const { return m_text; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

signals:
    void clicked();
    void textAnimationComplete(); // 添加动画完成信号


private:
    OutlineLabel* m_name;
    OutlineTextBrowser* m_text;
};
