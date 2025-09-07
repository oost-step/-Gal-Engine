// ResourceManager.h
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QSet>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

class ResourceManager : public QObject
{
    Q_OBJECT
public:
    static ResourceManager& instance();

    void preloadImage(const QString& path);
    void preloadImages(const QStringList& paths);
    QPixmap getPixmap(const QString& path) const;
    bool hasPixmap(const QString& path) const;

    void registerAudio(const QString& path);
    bool hasAudio(const QString& path) const;

    // JSON 文件加载功能
    QJsonDocument loadJsonDocument(const QString& path) const;
    QJsonObject loadJsonObject(const QString& path) const;
    QString loadTextFile(const QString& path) const;

signals:
    void imageLoaded(const QString& path);

private:
    explicit ResourceManager(QObject* parent = nullptr);
    QHash<QString, QPixmap> m_pixmaps;
    QSet<QString> m_audioPaths;
};

#endif // RESOURCEMANAGER_H