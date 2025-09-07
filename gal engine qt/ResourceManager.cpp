// ResourceManager.cpp
#include "ResourceManager.h"
#include <QFileInfo>
#include <QFile>
#include <QDebug>

ResourceManager::ResourceManager(QObject* parent) : QObject(parent) {}

ResourceManager& ResourceManager::instance() {
    static ResourceManager inst;
    return inst;
}

void ResourceManager::preloadImage(const QString& path) {
    if (path.isEmpty()) return;
    if (m_pixmaps.contains(path)) return;

    QPixmap px;
    if (QFileInfo::exists(path)) {
        if (px.load(path)) {
            m_pixmaps.insert(path, px);
            emit imageLoaded(path);
        }
        else {
            qDebug() << "Failed to load image:" << path;
        }
    }
    else {
        qDebug() << "Image file does not exist:" << path;
    }
}

void ResourceManager::preloadImages(const QStringList& paths) {
    for (const auto& p : paths) preloadImage(p);
}

QPixmap ResourceManager::getPixmap(const QString& path) const {
    auto it = m_pixmaps.find(path);
    if (it != m_pixmaps.end()) {
        return it.value();
    }

    // 如果图片未预加载，尝试直接加载
    QPixmap px;
    if (QFileInfo::exists(path) && px.load(path)) {
        // 注意：这里违反了const约定，但为了修复问题暂时这样做
        const_cast<ResourceManager*>(this)->m_pixmaps.insert(path, px);
        return px;
    }

    qDebug() << "Failed to get pixmap:" << path;
    return QPixmap();
}

bool ResourceManager::hasPixmap(const QString& path) const {
    return m_pixmaps.contains(path);
}

void ResourceManager::registerAudio(const QString& path) {
    if (path.isEmpty()) return;
    m_audioPaths.insert(path);
}

bool ResourceManager::hasAudio(const QString& path) const {
    return m_audioPaths.contains(path);
}

QJsonDocument ResourceManager::loadJsonDocument(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open JSON file:" << path;
        return QJsonDocument();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << error.errorString() << "in file:" << path;
        return QJsonDocument();
    }

    return doc;
}

QJsonObject ResourceManager::loadJsonObject(const QString& path) const {
    QJsonDocument doc = loadJsonDocument(path);
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "JSON document is not an object or is null:" << path;
        return QJsonObject();
    }

    return doc.object();
}

QString ResourceManager::loadTextFile(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open text file:" << path;
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    return content;
}