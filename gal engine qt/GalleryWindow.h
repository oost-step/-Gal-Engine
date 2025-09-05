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
    void onCgGame();       // ��� CG ��ť
    void onHsceneGame();   // ��� Hscene ��ť
    void onStandGame();    // ��� Stand ��ť
    void onReturnGame();   // ��� ���� ��ť
    void showPrevImage();  // ��ʾ��һ��
    void showNextImage();  // ��ʾ��һ��
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

    QScrollArea* previewArea;   // Ԥ����������
    QWidget* previewWidget;
    QHBoxLayout* previewLayout; // Ԥ������ͼ����

    QPixmap logoPixmap;

    QStringList imageList; // ��ǰ���ص�ͼƬ·��
    int currentIndex = -1; // ��ǰ��ʾ��ͼƬ����

    void loadImages(const QString& folder);
    void clearImages();
    void updateDisplay();
    void updatePreview();
};

#endif // GALLERYWINDOW_H