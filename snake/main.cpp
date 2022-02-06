#include "snakeclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    if(QT_VERSION>=QT_VERSION_CHECK(5,6,0))
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    SnakeClient client("192.168.2.4", 9000, 9001);

    return a.exec();
}
