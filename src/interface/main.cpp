#include "window.h"
#include <QApplication>
#include "mainwindow.h"

 int main(int argc,char *argv[])
{
    QApplication app(argc,argv);
//    Window window;
//    window.show();

    MainWindow m;
    m.show();
    return app.exec();
}
