#include "DialogueBox.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include "OutlineLabel.h"
#include "OutlineTextBrowser.h"

DialogueBox::DialogueBox(QWidget* parent) : QFrame(parent) {
    setObjectName("DialogueBox");
    setStyleSheet("#DialogueBox { background: rgba(0,0,0,180); border-radius: 12px; }"
        "QLabel { color: white; font-weight: bold; }"
        "QTextBrowser { background: transparent; color: white; border: none; font-size: 16px; }");

    connect(m_text, &OutlineTextBrowser::animationCompleteClicked,
        this, &DialogueBox::clicked);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(20 + 260, 16, 20, 16);
    m_name = new OutlineLabel(this);
    m_text = new OutlineTextBrowser(this);
    m_text->setDisplayDelay(80);

    connect(m_text, &OutlineTextBrowser::clicked, this, &DialogueBox::clicked);

    //m_text->setCharDelay(5);
    //m_text->setWordWrap(true);
    //m_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //m_name->setAttribute(Qt::WA_TransparentForMouseEvents);
    //m_text->setAttribute(Qt::WA_TransparentForMouseEvents);

    /*
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(m_name);
    effect->setBlurRadius(5);        // 模糊半径为0，就是硬边
    effect->setColor(Qt::yellow);     // 轮廓颜色
    effect->setOffset(0, 0);         // 偏移（可以尝试多加几个方向来模拟完整描边）

    m_name->setGraphicsEffect(effect);

    QGraphicsDropShadowEffect* eeffect = new QGraphicsDropShadowEffect(m_text);
    eeffect->setBlurRadius(5);        // 模糊半径为0，就是硬边
    eeffect->setColor(Qt::yellow);     // 轮廓颜色
    eeffect->setOffset(0, 0);         // 偏移（可以尝试多加几个方向来模拟完整描边）
    
    m_text->setGraphicsEffect(eeffect);
    */

    lay->addWidget(m_name);
    lay->addWidget(m_text, 1);
}

void DialogueBox::setSpeaker(const QString& name) { m_name->setText(name); }
void DialogueBox::setText(const QString& t) { m_text->setText(t); }

/*
void DialogueBox::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton) emit clicked();
    QFrame::mousePressEvent(ev);
}
*/
