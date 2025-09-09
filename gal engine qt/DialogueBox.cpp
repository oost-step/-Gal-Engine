#include "DialogueBox.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include "OutlineLabel.h"
#include "OutlineTextBrowser.h"
#include "ResourceManager.h"

DialogueBox::DialogueBox(QWidget* parent) : QFrame(parent) {
    setObjectName("DialogueBox");
    // �Ƴ�������ɫ���ã�����paintEvent�л���
    setStyleSheet("#DialogueBox { border-radius: 12px; background: transparent; }"
        "QLabel { color: white; font-weight: bold; }"
        "QTextBrowser { background: transparent; color: white; border: none; font-size: 16px; }");

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(20 + 260, 16, 20, 16);

    // ���ÿؼ�֮��ļ��
    lay->setSpacing(15); // ���ӿؼ�֮��Ĵ�ֱ���

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

    // Ԥ���ر���ͼƬ
    ResourceManager::instance().preloadImage("resources/dialoguebox.png");
}

void DialogueBox::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // ʹ��ResourceManager��ȡ����ͼƬ
    QPixmap bg = ResourceManager::instance().getPixmap("resources/dialoguebox.png");

    if (!bg.isNull()) {
        // ���Ʊ���ͼƬ����ȫ���ǶԻ�������
        painter.drawPixmap(0, 0, width(), height(), bg);
    }
    else {
        // ���ͼƬ����ʧ�ܣ�ʹ��Ĭ�ϱ���ɫ
        painter.fillRect(rect(), QColor(0, 0, 0, 180));
    }

    // ���û����paintEvent��ȷ���߿������������������
    QFrame::paintEvent(event);
}

void DialogueBox::resizeEvent(QResizeEvent* event) {
    QFrame::resizeEvent(event);
    // ǿ���ػ���ȷ������ͼƬ��Ӧ�µĴ�С
    update();
}

void DialogueBox::setSpeaker(const QString& name) { m_name->setText(name); }
void DialogueBox::setText(const QString& t) { m_text->setText(t); }