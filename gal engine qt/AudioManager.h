// AudioManager.h
#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QString>

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject* parent = nullptr);
    
    void setBgmVolume(float v);
    void playBgm(const QString& file);
    void stopBgm();
    void playSe(const QString& file);
    
    // 新增功能：播放音效并连接完成信号
    void playSeWithCallback(const QString& file, std::function<void()> callback);
    void disconnectSeCallbacks();

signals:
    void seFinished();

private:
    QMediaPlayer* m_bgm;
    QAudioOutput* m_bgmOut;
    QString m_currentBgm;
    
    QMediaPlayer* m_se;
    QAudioOutput* m_seOut;
    
    QList<std::function<void()>> m_seCallbacks;
};

#endif // AUDIOMANAGER_H