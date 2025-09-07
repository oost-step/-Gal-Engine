#pragma once
#include <QObject>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QMap>
#include <QStringList>
#include <QFileInfoList>

class ResourceManager : public QObject {
    Q_OBJECT
public:
    static ResourceManager& instance();

    static constexpr bool USE_PACKED_RESOURCES = true;
    // 🔹 写死的开关：true=打包模式，false=明文模式

    void preloadImage(const QString& path);
    void preloadImages(const QStringList& paths);
    QPixmap getPixmap(const QString& path) const;
    bool hasPixmap(const QString& path) const;

    void registerAudio(const QString& path);
    bool hasAudio(const QString& path) const;

    QJsonDocument loadJsonDocument(const QString& path) const;
    QJsonObject loadJsonObject(const QString& path) const;
    QString loadTextFile(const QString& path) const;

    bool loadPackage(const QString& filename);

    QByteArray getData(const QString& path) const;

    QFileInfoList getFileList(const QString& directory,
        const QStringList& filters = QStringList(),
        bool recursive = false);

signals:
    void imageLoaded(const QString& path);

private:
    explicit ResourceManager(QObject* parent = nullptr);
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    
    

    mutable QMap<QString, QPixmap> m_pixmaps;
    QSet<QString> m_audioPaths;

    // 存放打包文件解出的资源
    QMap<QString, QByteArray> m_resources;

    
    QString normalizePath(const QString& path) const;
    QByteArray xorDecrypt(const QByteArray& data) const;
    QByteArray zlibUncompress(const QByteArray& data) const;
};
