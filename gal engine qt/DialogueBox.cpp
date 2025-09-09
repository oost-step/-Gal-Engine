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
    // 移除背景颜色设置，将在paintEvent中绘制
    setStyleSheet("#DialogueBox { border-radius: 12px; background: transparent; }"
        "QLabel { color: white; font-weight: bold; }"
        "QTextBrowser { background: transparent; color: white; border: none; font-size: 16px; }");

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(20 + 260, 16, 20, 16);

    // 设置控件之间的间距
    lay->setSpacing(15); // 增加控件之间的垂直间距

    m_name = new OutlineLabel(this);
    m_text = new OutlineTextBrowser(this);
    m_text->setDisplayDelay(80);

    // 连接点击信号
    connect(m_text, &OutlineTextBrowser::clicked, this, &DialogueBox::clicked);

    // 连接动画完成点击信号
    connect(m_text, &OutlineTextBrowser::animationCompleteClicked,
        this, &DialogueBox::clicked);

    // 连接动画完成信号并转发
    connect(m_text, &OutlineTextBrowser::animationComplete,
        this, &DialogueBox::textAnimationComplete);

    lay->addWidget(m_name);
    lay->addWidget(m_text, 1);

    // 预加载背景图片
    ResourceManager::instance().preloadImage("resources/dialoguebox.png");
}

void DialogueBox::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 使用ResourceManager获取背景图片
    QPixmap bg = ResourceManager::instance().getPixmap("resources/dialoguebox.png");

    if (!bg.isNull()) {
        // 绘制背景图片，完全覆盖对话框区域
        painter.drawPixmap(0, 0, width(), height(), bg);
    }
    else {
        // 如果图片加载失败，使用默认背景色
        painter.fillRect(rect(), QColor(0, 0, 0, 180));
    }

    // 调用基类的paintEvent以确保边框等其他绘制正常进行
    QFrame::paintEvent(event);
}

void DialogueBox::resizeEvent(QResizeEvent* event) {
    QFrame::resizeEvent(event);
    // 强制重绘以确保背景图片适应新的大小
    update();
}

void DialogueBox::setSpeaker(const QString& name) { m_name->setText(name); }
void DialogueBox::setText(const QString& t) { m_text->setText(t); }