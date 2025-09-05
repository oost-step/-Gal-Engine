#pragma once
#include <QObject>
#include <QPointer>
#include <QString>

class QMediaPlayer;
class QAudioOutput;

class AudioManager : public QObject {
    Q_OBJECT
public:
    explicit AudioManager(QObject* parent = nullptr);
    void setBgmVolume(float v);

public slots:
    void playBgm(const QString& file);
    void stopBgm();
    void playSe(const QString& file);

private:
    QPointer<QMediaPlayer> m_bgm;
    QPointer<QAudioOutput> m_bgmOut;
    QPointer<QMediaPlayer> m_se;
    QPointer<QAudioOutput> m_seOut;
    QString m_currentBgm;
};
