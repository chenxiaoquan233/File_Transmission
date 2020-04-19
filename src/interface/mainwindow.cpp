#include "mainwindow.h"
#include <QHBoxLayout>
#include "window.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setSpacing(5);

    layout->setMargin(0);

    Window *w1 = new Window;

    layout->addWidget(w1);
    this->resize(400, 600);
}
