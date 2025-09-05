#include <QWidget>
#include <QPixmap>
#include <QPointer>
#include <QString>

class QMediaPlayer;
class QAudioOutput;
class QLabel;
class QPushButton;
class MainWindow;
class SettingWindow;
class GalleryWindow;

class StartWindow : public QWidget {
    Q_OBJECT
public:
    explicit StartWindow(QWidget* parent = nullptr);
    ~StartWindow();

private slots:
    void onStartGame();
    void onContinueGame();
    void onGalleryGame();
    void onSettingGame();
    void onExitGame();

private:
    void paintEvent(QPaintEvent* event);

    QPointer<QMediaPlayer> mm_bgm;
    QPointer<QAudioOutput> mm_bgmOut;
    QPointer<QMediaPlayer> mm_se;
    QPointer<QAudioOutput> mm_seOut;

    QLabel* logoLabel;
    QPushButton* startBtn;
    QPushButton* continueBtn;
    QPushButton* galleryBtn;
    QPushButton* settingBtn;
    QPushButton* exitBtn;
    MainWindow* m_mainWindow = nullptr;
    SettingWindow* m_settingWindow = nullptr;
    GalleryWindow* m_galleryWindow = nullptr;

    QPixmap logoPixmap;
};