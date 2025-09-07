#include "ResourceManager.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QDirIterator>
#include <zlib.h>

ResourceManager::ResourceManager(QObject* parent) : QObject(parent) {}

ResourceManager& ResourceManager::instance() {
    static ResourceManager inst;
    return inst;
}

bool ResourceManager::loadPackage(const QString& filename) {
    if (!USE_PACKED_RESOURCES) return true; // 调试模式不加载

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray all = f.readAll();
    f.close();

    const uchar* p = (const uchar*)all.constData();
    int pos = 0;

    auto readInt = [&](int& value) {
        if (pos + 4 > all.size()) return false;
        value = *(int*)(p + pos);
        pos += 4;
        return true;
    };

    int fileCount = 0;
    if (!readInt(fileCount)) return false;

    for (int i = 0; i < fileCount; i++) {
        int nameLen;
        if (!readInt(nameLen)) return false;
        QByteArray name((const char*)(p + pos), nameLen);
        pos += nameLen;

        int dataLen;
        if (!readInt(dataLen)) return false;
        QByteArray encData((const char*)(p + pos), dataLen);
        pos += dataLen;

        QByteArray decData = xorDecrypt(encData);
        QByteArray uncompressed = zlibUncompress(decData);

        m_resources.insert(QString::fromUtf8(name), uncompressed);
    }
    return true;
}

void ResourceManager::preloadImage(const QString& path) {
    if (path.isEmpty()) return;
    if (m_pixmaps.contains(path)) return;

    QPixmap px;
    QByteArray data = getData(path);
    if (!data.isEmpty() && px.loadFromData(data)) {
        m_pixmaps.insert(path, px);
        emit imageLoaded(path);
    }
    else {
        qDebug() << "Failed to load image:" << path;
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

    QPixmap px;
    QByteArray data = getData(path);
    if (!data.isEmpty() && px.loadFromData(data)) {
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
    QByteArray data = getData(path);
    if (data.isEmpty()) {
        qDebug() << "Failed to open JSON file:" << path;
        return QJsonDocument();
    }

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
    QByteArray data = getData(path);
    if (data.isEmpty()) {
        qDebug() << "Failed to open text file:" << path;
        return QString();
    }
    return QString::fromUtf8(data);
}

QByteArray ResourceManager::getData(const QString& path) const {
    if (USE_PACKED_RESOURCES) {
        qDebug() << "[ResourceManager] lookup:" << path << "normalized:" << normalizePath(path)
            << "exists:" << m_resources.contains(normalizePath(path));
        return m_resources.value(normalizePath(path));
    }
    else {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return QByteArray();
        qDebug() << "[ResourceManager] lookup:" << path << "normalized:" << normalizePath(path)
            << "exists:" << m_resources.contains(normalizePath(path));
        return f.readAll();
    }
}

QString ResourceManager::normalizePath(const QString& path) const {
    QString p = QDir::cleanPath(QDir::fromNativeSeparators(path));

    // 去掉前导 "./"
    if (p.startsWith("./")) p = p.mid(2);

    int idx = p.indexOf("assets/");
    if (idx == -1) idx = p.indexOf("resources/");
    if (idx != -1) {
        return p.mid(idx);
    }
    return p;
}

QByteArray ResourceManager::xorDecrypt(const QByteArray& data) const {
    QByteArray out = data;
    for (int i = 0; i < out.size(); i++) {
        out[i] = out[i] ^ 0x5A;
    }
    return out;
}

QByteArray ResourceManager::zlibUncompress(const QByteArray& data) const {
    QByteArray result;
    int bufferSize = 1024 * 1024;
    result.resize(bufferSize);

    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    inflateInit(&strm);

    strm.next_in = (Bytef*)data.data();
    strm.avail_in = data.size();

    int ret;
    do {
        strm.next_out = (Bytef*)(result.data() + strm.total_out);
        strm.avail_out = result.size() - strm.total_out;
        ret = inflate(&strm, Z_NO_FLUSH);

        if (ret == Z_BUF_ERROR || strm.avail_out == 0) {
            result.resize(result.size() * 2);
            continue;
        }
        if (ret == Z_STREAM_END) break;
    } while (ret == Z_OK);

    inflateEnd(&strm);
    result.resize(strm.total_out);
    return result;
}
QFileInfoList ResourceManager::getFileList(const QString& directory,
    const QStringList& filters,
    bool recursive) {
    QFileInfoList result;

    if (USE_PACKED_RESOURCES) {
        // 打包资源模式：从已加载的资源中查找
        QString normalizedDir = normalizePath(directory);
        if (!normalizedDir.endsWith('/')) {
            normalizedDir += '/';
        }

        // 遍历所有资源，查找匹配的文件
        for (auto it = m_resources.keyBegin(); it != m_resources.keyEnd(); ++it) {
            QString filePath = *it;

            // 检查是否在指定目录下
            if (filePath.startsWith(normalizedDir)) {
                // 检查是否匹配过滤器
                if (filters.isEmpty() ||
                    QDir::match(filters, QFileInfo(filePath).fileName())) {
                    // 创建QFileInfo对象（注意：在资源包模式下，这些信息可能不完整）
                    QFileInfo fileInfo;
                    fileInfo.setFile(filePath); // 只设置文件路径
                    result.append(fileInfo);
                }
            }
        }
    }
    else {
        // 非打包资源模式：直接从文件系统读取
        QDir dir(directory);

        if (recursive) {
            // 递归获取所有子目录的文件
            dir.setNameFilters(filters);
            dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
            result = dir.entryInfoList();

            // 获取所有子目录
            QDirIterator it(directory, QDir::Dirs | QDir::NoDotAndDotDot,
                QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString subDir = it.next();
                QDir subQDir(subDir);
                result.append(subQDir.entryInfoList(filters, QDir::Files));
            }
        }
        else {
            // 非递归获取文件
            result = dir.entryInfoList(filters, QDir::Files);
        }
    }

    return result;
}