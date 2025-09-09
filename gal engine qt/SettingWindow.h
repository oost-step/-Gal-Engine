#pragma once
#include <QWidget>
#include <QPixmap>
#include <QPointer>
#include <QString>
#include "AudioManager.h"
#include "MainWindow.h"

class QMediaPlayer;
class QAudioOutput;
class QLabel;
class QPushButton;
class MainWindow;
class StartWindow;
class QPixmap;
class AudioManager;

extern bool g_autoMode;
extern bool g_skipMode;

class SettingWindow : public QWidget {
    Q_OBJECT
public:
    explicit SettingWindow(QWidget* parent = nullptr, bool fromMain = false, QWidget* caller = nullptr);
    ~SettingWindow();

    void setCaller(QWidget* caller);

signals:
    void closedFromMainWindow();   // 从 MainWindow 打开的 Setting 关闭时发出
    void closedFromStartWindow();  // 从 StartWindow 打开的 Setting 关闭时发出
    void modesChanged(bool autoMode, bool skipMode); // 当模式变化时发出
    void closed();

private slots:
    void onReturnGame();

private:
    void paintEvent(QPaintEvent* event);

    QPointer<QMediaPlayer> mm_se;
    QPointer<QAudioOutput> mm_seOut;
    QPointer<QMediaPlayer> mm_bgm;
    QPointer<QAudioOutput> mm_bgmOut;

    QLabel* logoLabel;
    QPushButton* returnBtn = nullptr;
    QPointer<QWidget> m_caller; // 安全指针，若 caller 被销毁会自动变 null

    AudioManager* m_audioManager = nullptr;

    QPixmap logoPixmap;

    bool m_fromMainWindow = false;
};