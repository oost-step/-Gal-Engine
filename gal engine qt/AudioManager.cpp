#include "AudioManager.h"
#include "ResourceManager.h"
#include <QUrl>
#include <QDebug>
#include <QBuffer>

AudioManager::AudioManager(QObject* parent) : QObject(parent) {
    m_bgm = new QMediaPlayer(this);
    m_bgmOut = new QAudioOutput(this);
    m_bgm->setAudioOutput(m_bgmOut);
    m_bgmOut->setVolume(0.6f);

    m_se = new QMediaPlayer(this);
    m_seOut = new QAudioOutput(this);
    m_se->setAudioOutput(m_seOut);
    m_seOut->setVolume(1.0f);

    connect(m_se, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            emit seFinished();

            for (const auto& callback : m_seCallbacks) {
                if (callback) callback();
            }
            m_seCallbacks.clear();
        }
    });
}

void AudioManager::setBgmVolume(float v) {
    if (m_bgmOut) m_bgmOut->setVolume(qBound(0.0f, v, 1.0f));
}

void AudioManager::playBgm(const QString& file) {
    if (file.isEmpty()) return;
    if (m_currentBgm == file && m_bgm->playbackState() == QMediaPlayer::PlayingState) return;
    m_currentBgm = file;

    if (ResourceManager::USE_PACKED_RESOURCES) {
        QByteArray data = ResourceManager::instance().getData(file);
        if (data.isEmpty()) {
            qDebug() << "BGM not found in resources:" << file;
            return;
        }
        QBuffer* buffer = new QBuffer(m_bgm);
        buffer->setData(data);
        buffer->open(QIODevice::ReadOnly);
        m_bgm->setSourceDevice(buffer);
    }
    else {
        m_bgm->setSource(QUrl::fromLocalFile(file));
    }

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

    if (ResourceManager::USE_PACKED_RESOURCES) {
        QByteArray data = ResourceManager::instance().getData(file);
        if (data.isEmpty()) {
            qDebug() << "SE not found in resources:" << file;
            return;
        }
        QBuffer* buffer = new QBuffer(m_se);
        buffer->setData(data);
        buffer->open(QIODevice::ReadOnly);
        m_se->setSourceDevice(buffer);
    }
    else {
        m_se->setSource(QUrl::fromLocalFile(file));
    }

    m_se->setLoops(1);
    m_se->play();
}

void AudioManager::playSeWithCallback(const QString& file, std::function<void()> callback) {
    if (file.isEmpty()) return;
    if (m_se->playbackState() == QMediaPlayer::PlayingState) m_se->stop();

    if (callback) {
        m_seCallbacks.append(callback);
    }

    if (ResourceManager::USE_PACKED_RESOURCES) {
        QByteArray data = ResourceManager::instance().getData(file);
        if (data.isEmpty()) {
            qDebug() << "SE not found in resources:" << file;
            return;
        }
        QBuffer* buffer = new QBuffer(m_se);
        buffer->setData(data);
        buffer->open(QIODevice::ReadOnly);
        m_se->setSourceDevice(buffer);
    }
    else {
        m_se->setSource(QUrl::fromLocalFile(file));
    }

    m_se->setLoops(1);
    m_se->play();
}

void AudioManager::disconnectSeCallbacks() {
    m_seCallbacks.clear();
}
