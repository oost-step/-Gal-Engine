#pragma once
#include <QWidget>
#include <QPixmap>
#include <QPointer>
#include <QString>

class QMediaPlayer;
class QAudioOutput;
class QLabel;
class QPushButton;
class MainWindow;
class StartWindow;

class SettingWindow : public QWidget {
    Q_OBJECT
public:
    explicit SettingWindow(QWidget* parent = nullptr);
    ~SettingWindow();

private slots:
    void onReturnGame();

private:
    void paintEvent(QPaintEvent* event);

    QPointer<QMediaPlayer> mm_se;
    QPointer<QAudioOutput> mm_seOut;
    QPointer<QMediaPlayer> mm_bgm;
    QPointer<QAudioOutput> mm_bgmOut;

    QLabel* logoLabel;
    QPushButton* returnBtn;
    StartWindow* m_startWindow = nullptr;

    QPixmap logoPixmap;
};