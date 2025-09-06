#include <QApplication>
#include <QFont>
#include "StartWindow.h"

int main(int argc, char* argv[]) {
    qputenv("QT_MEDIA_BACKEND", "windows");
    QApplication a(argc, argv);

    QFont font("Simhei", 18);
    a.setFont(font);

    StartWindow w;
    w.show();
    return a.exec();
}
