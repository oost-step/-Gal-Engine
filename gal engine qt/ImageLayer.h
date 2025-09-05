#pragma once
#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>

class ImageLayer : public QWidget {
    Q_OBJECT
public:
    explicit ImageLayer(QWidget* parent = nullptr);
    void setSprite(const QString& slot, const QPixmap& px, int fadeInDuration = 500);
    void clearSprite(const QString& slot, int fadeOutDuration = 500);
    void clearAll();

protected:
    void resizeEvent(QResizeEvent* ev) override;

private:
    QLabel* pick(const QString& slot);
    void layoutSprites();

    QMap<QLabel*, QPropertyAnimation*> m_animations;
    int m_defaultFadeDuration = 1000;

    QLabel* m_left;
    QLabel* m_right;
    QLabel* m_pleft;
    QLabel* m_pcenter;
    QLabel* m_pright;
};

class ImageLayerTop : public QWidget {
    Q_OBJECT
public:
    explicit ImageLayerTop(QWidget* parent = nullptr);
    void setSpriteTop(const QString& slot, const QPixmap& px);
    void clearSpriteTop(const QString& slot);
    void clearAllTop();

protected:
    void resizeEvent(QResizeEvent* ev) override;

private:
    QLabel* pick(const QString& slot);
    void layoutSprites();

    QLabel* m_left;
    QLabel* m_right;
    QLabel* m_pleft;
    QLabel* m_pcenter;
    QLabel* m_pright;
};
