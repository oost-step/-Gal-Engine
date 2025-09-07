#include "ImageLayer.h"
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

ImageLayer::ImageLayer(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_lleft = new QLabel(this);
    m_left = new QLabel(this);
    m_right = new QLabel(this);
    m_pleft = new QLabel(this);
    m_pcenter = new QLabel(this);
    m_pright = new QLabel(this);

    for (auto* l : { m_lleft, m_left, m_right, m_pleft, m_pcenter, m_pright }) {
        l->setScaledContents(false);
        l->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        l->hide();
    }

}

QLabel* ImageLayer::pick(const QString& slot) {
    if (slot.compare("left", Qt::CaseInsensitive) == 0) return m_lleft;
    if (slot.compare("center", Qt::CaseInsensitive) == 0) return m_left;
    if (slot.compare("right", Qt::CaseInsensitive) == 0) return m_right;
    if (slot.compare("pleft", Qt::CaseInsensitive) == 0) return m_pleft;
    if (slot.compare("pright", Qt::CaseInsensitive) == 0) return m_pright;
    if (slot.compare("pcenter", Qt::CaseInsensitive) == 0) return m_pcenter;
    return m_pleft;
}

void ImageLayer::setSprite(const QString& slot, const QPixmap& px, int fadeInDuration)
{
    QLabel* lbl = pick(slot);
    if (!lbl) return;

    // 如果已经有动画在运行，先停止它
    if (m_animations.contains(lbl)) {
        QPropertyAnimation* oldAnim = m_animations.value(lbl);
        if (oldAnim && oldAnim->state() == QPropertyAnimation::Running) {
            oldAnim->stop();
        }
        m_animations.remove(lbl);
    }

    // 设置初始透明度为0（完全透明）
    lbl->setPixmap(px);
    lbl->setGraphicsEffect(nullptr); // 清除之前的特效

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(lbl);
    effect->setOpacity(0.0); // 初始完全透明
    lbl->setGraphicsEffect(effect);
    lbl->show();
    layoutSprites();

    // 创建淡入动画
    QPropertyAnimation* fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(fadeInDuration);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InOutQuad);

    // 动画完成后清理
    connect(fadeIn, &QPropertyAnimation::finished, [lbl, effect, this]() {
        // 动画完成后移除特效，恢复正常显示
        lbl->setGraphicsEffect(nullptr);
        //delete effect;
        m_animations.remove(lbl);
    });

    m_animations.insert(lbl, fadeIn);
    fadeIn->start();
}

void ImageLayer::clearSprite(const QString& slot, int fadeOutDuration)
{
    QLabel* lbl = pick(slot);
    if (!lbl || !lbl->isVisible() || lbl->pixmap().isNull()) {
        // 如果label不存在、不可见或没有图片，直接清理
        if (lbl) {
            lbl->clear();
            lbl->hide();
        }
        return;
    }

    // 如果已经有动画在运行，先停止它
    if (m_animations.contains(lbl)) {
        QPropertyAnimation* oldAnim = m_animations.value(lbl);
        if (oldAnim && oldAnim->state() == QPropertyAnimation::Running) {
            oldAnim->stop();
            oldAnim->deleteLater();
        }
        m_animations.remove(lbl);
    }

    // 获取当前的特效或创建新的
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(lbl->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(lbl);
        effect->setOpacity(1.0); // 初始完全不透明
        lbl->setGraphicsEffect(effect);
    }

    // 创建淡出动画
    QPropertyAnimation* fadeOut = new QPropertyAnimation(effect, "opacity", this);
    fadeOut->setDuration(fadeOutDuration);
    fadeOut->setStartValue(effect->opacity()); // 从当前透明度开始
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InOutQuad);

    // 动画完成后清理
    connect(fadeOut, &QPropertyAnimation::finished, this, [=]() {
        lbl->clear();
        lbl->hide();
        lbl->setGraphicsEffect(nullptr); // 移除特效

        if (m_animations.value(lbl) == fadeOut) {
            m_animations.remove(lbl);
        }
        //fadeOut->deleteLater();
    });

    m_animations.insert(lbl, fadeOut);
    fadeOut->start();
}

void ImageLayer::clearAll() {
    for (auto* l : { m_lleft, m_left, m_right, m_pleft, m_pcenter, m_pright }) { l->clear(); l->hide(); }
}

void ImageLayer::resizeEvent(QResizeEvent* ev) {
    QWidget::resizeEvent(ev);
    layoutSprites();
}

void ImageLayer::layoutSprites() {
    const int w = width();
    const int h = height();
    const int baseW = w / 3;
    const int baseH = h;
    m_lleft->setGeometry(260, 0, 510, 720);
    m_left->setGeometry(640 - 255, 0, 510, 720);
    //m_center->setGeometry((w - baseW) / 2, h - baseH, baseW, baseH);
    m_right->setGeometry(260 + 510, 0, 510, 720);
    m_pleft->setGeometry(0, 460, 260, 260);
    m_pcenter->setGeometry(260, 460, 260, 260);
    m_pright->setGeometry(260 + 510, 460, 260, 260);
}


ImageLayerTop::ImageLayerTop(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_lleft = new QLabel(this);
    m_left = new QLabel(this);
    m_right = new QLabel(this);
    m_pleft = new QLabel(this);
    m_pcenter = new QLabel(this);
    m_pright = new QLabel(this);

    for (auto* l : { m_lleft, m_left, m_right, m_pleft, m_pcenter, m_pright }) {
        l->setScaledContents(false);
        l->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        l->hide();
    }

}

QLabel* ImageLayerTop::pick(const QString& slot) {
    if (slot.compare("left", Qt::CaseInsensitive) == 0) return m_lleft;
    if (slot.compare("center", Qt::CaseInsensitive) == 0) return m_left;
    if (slot.compare("right", Qt::CaseInsensitive) == 0) return m_right;
    if (slot.compare("pleft", Qt::CaseInsensitive) == 0) return m_pleft;
    if (slot.compare("pright", Qt::CaseInsensitive) == 0) return m_pright;
    if (slot.compare("pcenter", Qt::CaseInsensitive) == 0) return m_pcenter;
    return m_pleft;
}

void ImageLayerTop::setSpriteTop(const QString& slot, const QPixmap& px) {
    QLabel* lbl = pick(slot);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    if (!lbl) return;
    lbl->setPixmap(px);
    raise();
    lbl->show();
    layoutSprites();
}

void ImageLayerTop::clearSpriteTop(const QString& slot) {
    QLabel* lbl = pick(slot);
    if (!lbl) return;
    lbl->clear();
    lbl->hide();
}

void ImageLayerTop::clearAllTop() {
    for (auto* l : { m_lleft, m_left, m_right, m_pleft, m_pcenter, m_pright }) { l->clear(); l->hide(); }
}

void ImageLayerTop::resizeEvent(QResizeEvent* ev) {
    QWidget::resizeEvent(ev);
    layoutSprites();
}

void ImageLayerTop::layoutSprites() {
    const int w = width();
    const int h = height();
    const int baseW = w / 3;
    const int baseH = h;
    m_left->setGeometry(260, 0, 510, 720);
    m_left->setGeometry(640 - 255, 0, 510, 720);
    //m_center->setGeometry((w - baseW) / 2, h - baseH, baseW, baseH);
    m_right->setGeometry(260 + 510, 0, 510, 720);
    m_pleft->setGeometry(0, 460, 260, 260);
    m_pcenter->setGeometry(260, 460, 260, 260);
    m_pright->setGeometry(260 + 510, 460, 260, 260);
}