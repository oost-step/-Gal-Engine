#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QWidget>
#include <QPixmap>

class MainWindow;
class SettingWindow;
class GalleryWindow;
class AudioManager;

class StartWindow : public QWidget
{
    Q_OBJECT
public:
    explicit StartWindow(QWidget* parent = nullptr);
    ~StartWindow();

    int hidden = 0;

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onStartGame();
    void onContinueGame();
    void onGalleryGame();
    void onSettingGame();
    void onExitGame();

private:
    QPixmap logoPixmap;

    class QPushButton* startBtn;
    class QPushButton* continueBtn;
    class QPushButton* galleryBtn;
    class QPushButton* settingBtn;
    class QPushButton* exitBtn;

    MainWindow* m_mainWindow = nullptr;
    SettingWindow* m_settingWindow = nullptr;
    GalleryWindow* m_galleryWindow = nullptr;

    AudioManager* m_audioManager = nullptr;

    void loadHidden();
};

#endif // STARTWINDOW_H