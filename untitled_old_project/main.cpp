#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,9,0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    qRegisterMetaType<QVector<double>>("QVector<double>");
    Widget w;
    w.show();
    return a.exec();
}
