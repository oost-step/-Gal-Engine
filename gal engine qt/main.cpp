#include <QApplication>
#include <QFont>
#include "StartWindow.h"
#include "ResourceManager.h"

int main(int argc, char* argv[]) {

    if (ResourceManager::USE_PACKED_RESOURCES) {
        ResourceManager::instance().loadPackage("resources.pak");
    }

    qputenv("QT_MEDIA_BACKEND", "windows");
    QApplication a(argc, argv);

    QFont font("Simhei", 18);
    a.setFont(font);

    StartWindow w;
    w.show();
    return a.exec();
}
