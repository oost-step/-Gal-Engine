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

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(20 + 260, 16, 20, 16);
    m_name = new OutlineLabel(this);
    m_text = new OutlineTextBrowser(this);
    m_text->setDisplayDelay(80);

    // ���ӵ���ź�
    connect(m_text, &OutlineTextBrowser::clicked, this, &DialogueBox::clicked);

    // ���Ӷ�����ɵ���ź�
    connect(m_text, &OutlineTextBrowser::animationCompleteClicked,
        this, &DialogueBox::clicked);

    // ���Ӷ�������źŲ�ת��
    connect(m_text, &OutlineTextBrowser::animationComplete,
        this, &DialogueBox::textAnimationComplete);

    lay->addWidget(m_name);
    lay->addWidget(m_text, 1);
}

void DialogueBox::setSpeaker(const QString& name) { m_name->setText(name); }
void DialogueBox::setText(const QString& t) { m_text->setText(t); }