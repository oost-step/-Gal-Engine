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
    
    void playSeWithCallback(const QString& file, std::function<void()> callback);
    void disconnectSeCallbacks();

    bool isBgmPlaying() const;

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