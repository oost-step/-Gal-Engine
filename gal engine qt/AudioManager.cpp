#include "AudioManager.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QDebug>

AudioManager::AudioManager(QObject* parent) : QObject(parent) {
    m_bgm = new QMediaPlayer(this);
    m_bgmOut = new QAudioOutput(this);
    m_bgm->setAudioOutput(m_bgmOut);
    m_bgmOut->setVolume(0.6f);

    m_se = new QMediaPlayer(this);
    m_seOut = new QAudioOutput(this);
    m_se->setAudioOutput(m_seOut);
    m_seOut->setVolume(1.0f);
}

void AudioManager::setBgmVolume(float v) {
    if (m_bgmOut) m_bgmOut->setVolume(qBound(0.0f, v, 1.0f));
}

void AudioManager::playBgm(const QString& file) {
    if (file.isEmpty()) return;
    if (m_currentBgm == file && m_bgm->playbackState() == QMediaPlayer::PlayingState) return;
    m_currentBgm = file;
    m_bgm->setSource(QUrl::fromLocalFile(file));
    m_bgm->setLoops(QMediaPlayer::Infinite);
    m_bgm->play();
}

void AudioManager::stopBgm() {
    if (m_bgm) {
        m_bgm->stop();
        m_currentBgm.clear();
    }
}

void AudioManager::playSe(const QString& file) {
    if (file.isEmpty()) return;
    if (m_se->playbackState() == QMediaPlayer::PlayingState) m_se->stop();
    m_se->setSource(QUrl::fromLocalFile(file));
    m_se->setLoops(1);
    m_se->play();
}
