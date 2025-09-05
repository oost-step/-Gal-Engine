#include "ResourceManager.h"
#include <QFileInfo>

ResourceManager::ResourceManager(QObject* parent) : QObject(parent) {}

ResourceManager& ResourceManager::instance() {
    static ResourceManager inst;
    return inst;
}

void ResourceManager::preloadImage(const QString& path) {
    if (path.isEmpty()) return;
    if (m_pixmaps.contains(path)) return;
    QPixmap px;
    if (QFileInfo::exists(path)) px.load(path);
    if (!px.isNull()) {
        m_pixmaps.insert(path, px);
        emit imageLoaded(path);
    }
}

void ResourceManager::preloadImages(const QStringList& paths) {
    for (const auto& p : paths) preloadImage(p);
}

QPixmap ResourceManager::getPixmap(const QString& path) const {
    return m_pixmaps.value(path, QPixmap());
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
