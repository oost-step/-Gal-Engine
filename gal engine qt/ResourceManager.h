#pragma once
#include <QObject>
#include <QString>
#include <QPixmap>
#include <QMap>
#include <QSet>

class ResourceManager : public QObject {
    Q_OBJECT
public:
    explicit ResourceManager(QObject* parent = nullptr);
    static ResourceManager& instance();

    void preloadImage(const QString& path);
    void preloadImages(const QStringList& paths);
    QPixmap getPixmap(const QString& path) const;
    bool hasPixmap(const QString& path) const;

    void registerAudio(const QString& path);
    bool hasAudio(const QString& path) const;

signals:
    void imageLoaded(const QString& path);

private:
    QMap<QString, QPixmap> m_pixmaps;
    QSet<QString> m_audioPaths;
};
