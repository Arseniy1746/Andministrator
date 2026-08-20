#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Use a clean modern style where available
    a.setStyle(QStyleFactory::create("Fusion"));

    QFont font("Segoe UI", 10);
    a.setFont(font);

    a.setApplicationName("Messenger Server Admin");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("QtMessengerProject");

    MainWindow w;
    w.show();
    return a.exec();
}
