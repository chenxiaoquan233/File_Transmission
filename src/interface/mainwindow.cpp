#include "mainwindow.h"
#include <QHBoxLayout>
#include "window.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setSpacing(5);

    layout->setMargin(0);


    Window *w1 = new Window;
    Window *w2 = new Window;

    layout->addWidget(w1);

    layout->addWidget(w2);

}
