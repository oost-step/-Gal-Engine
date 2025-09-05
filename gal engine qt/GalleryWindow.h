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
    void onCgGame();
    void onHsceneGame();
    void onStandGame();
    void onReturnGame();
    void showPrevImage();
    void showNextImage();
    void onThumbnailClicked(int index);
    void onMusicGame();


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
    QPushButton* musicBtn;
    StartWindow* m_startWindow = nullptr;

    QScrollArea* previewArea;
    QWidget* previewWidget;
    QHBoxLayout* previewLayout;

    QPixmap logoPixmap;

    QStringList imageList;
    int currentIndex = -1;

    void loadImages(const QString& folder);
    void clearImages();
    void updateDisplay();
    void updatePreview();
    void loadMusic(const QString& folder);
};

#endif // GALLERYWINDOW_H