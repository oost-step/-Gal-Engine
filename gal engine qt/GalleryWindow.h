#ifndef GALLERYWINDOW_H
#define GALLERYWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPointer>

class QMediaPlayer;
class QAudioOutput;
class QLabel;
class QPushButton;
class MainWindow;
class SettingWindow;
class StartWindow;

class GalleryWindow : public QWidget {
    Q_OBJECT
public:
    explicit GalleryWindow(QWidget* parent = nullptr);
    ~GalleryWindow();

private slots:
    void onCgGame();       // 点击 CG 按钮
    void onHsceneGame();   // 点击 Hscene 按钮
    void onStandGame();    // 点击 Stand 按钮
    void onReturnGame();   // 点击 返回 按钮
    void showPrevImage();  // 显示上一张
    void showNextImage();  // 显示下一张
    void onThumbnailClicked(int index);

private:
    void paintEvent(QPaintEvent* event);

    QPointer<QMediaPlayer> mmm_se;
    QPointer<QAudioOutput> mmm_seOut;
    QPointer<QMediaPlayer> mmm_bgm;
    QPointer<QAudioOutput> mmm_bgmOut;

    QLabel* logoLabel;
    QLabel* displayLabel;

    QPushButton* cgBtn;
    QPushButton* standBtn;
    QPushButton* hsceneBtn;
    QPushButton* returnBtn;
    QPushButton* prevBtn;
    QPushButton* nextBtn;
    StartWindow* m_startWindow = nullptr;

    QScrollArea* previewArea;   // 预览滚动区域
    QWidget* previewWidget;
    QHBoxLayout* previewLayout; // 预览缩略图布局

    QPixmap logoPixmap;

    QStringList imageList; // 当前加载的图片路径
    int currentIndex = -1; // 当前显示的图片索引

    void loadImages(const QString& folder);
    void clearImages();
    void updateDisplay();
    void updatePreview();
};

#endif // GALLERYWINDOW_H