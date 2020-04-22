#include <QtGui>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTableWidget>
#include <QProgressDialog>
#include <QHeaderView>
#include <QMessageBox>
#include "window.h"

Window::Window(QWidget *parent) : QDialog(parent)
{
    connect_status = false;

    browseButton  = createButton(QStringLiteral("Browse..."), SLOT(browse()));
    sendButton    = createButton(QStringLiteral("Send")     , SLOT(try_send()));
    connectButton = createButton(QStringLiteral("Connect")  , SLOT(try_connect()));

    ipaddrComboBox = createComboBox("", true);
    ipaddrComboBox->setMinimumWidth(150);
    portComboBox = createComboBox("", true);
    portComboBox->setMinimumWidth(80);
    directoryComboBox = createComboBox("", false);

    ipaddrLabel        = new QLabel(tr("Host Addr:"));
    portLabel          = new QLabel(tr("Port:"));
    directoryLabel     = new QLabel(tr("In directory:"));
    connectStatusLabel = new QLabel(tr("not connected"));

    initFilesTable();

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addWidget(connectStatusLabel, 2, 0);

    mainLayout->addWidget(ipaddrLabel       , 3, 0);
    mainLayout->addWidget(ipaddrComboBox    , 3, 1, 1, 2);
    mainLayout->addWidget(portLabel         , 3, 3);
    mainLayout->addWidget(portComboBox      , 3, 4);
    mainLayout->addWidget(connectButton     , 3, 5);

    mainLayout->addWidget(directoryLabel    , 4, 0);
    mainLayout->addWidget(directoryComboBox , 4, 1, 1, 4);
    mainLayout->addWidget(browseButton      , 4, 5);

    mainLayout->addWidget(filesTable        , 5, 0, 1, 6);

    mainLayout->addWidget(sendButton        , 6, 5);

    setLayout(mainLayout);
    setWindowTitle(tr("Find Files"));
}

void Window::browse()
{
    QString directory = QFileDialog::getExistingDirectory(this, QObject::tr("Find Files"), "/");
    if (!directory.isEmpty())
    {
        directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->currentIndex() + 1);
        find();
    }
}

void Window::find()
{
    //empty the table at start
    filesTable->setRowCount(0);

    QString path = directoryComboBox->currentText();
    QDir directory = QDir(path);

    //directories and then files
    QStringList dirs  = directory.entryList(QStringList("*"), QDir::Dirs);
    QStringList files = directory.entryList(QStringList("*"), QDir::Files | QDir::NoSymLinks);
    showDirs(directory, dirs, 0);
    showFiles(directory, files, 0);
}

QStringList Window::findFiles(const QDir &directory,const QStringList &files, const QString &text)
{
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0,files.size());
    progressDialog.setWindowTitle(tr("Find Files"));
    QStringList foundFiles;
    for (int i = 0; i < files.size(); ++i) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Searching file number %1 of %2...")
                                    .arg(i).arg(files.size()));
        qApp->processEvents();
        if (progressDialog.wasCanceled()) break;
        QFile file(directory.absoluteFilePath(files[i]));
        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            QTextStream in(&file);

            while (!in.atEnd()) {
                if (progressDialog.wasCanceled()) break;
                line = in.readLine();
                if (line.contains(text)) {
                    foundFiles << files[i];
                    break;
                }
            }
        }
    }
    return foundFiles;
}

void Window::showFiles(const QDir &directory,const QStringList &files, int layers)
{
    for (int i = 0; i < files.size(); ++i)
    {
        QFile file(directory.absoluteFilePath(files[i]));

        double size = QFileInfo(file).size();
        int bs = 0;
        while(size > 1024)
        {
            size /=1024;
            bs ++;
        }
        string spaces;
        for(int i = 0; i < layers; ++i) spaces += "-";
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(QString::fromStdString(spaces) + files[i]);
        fileNameItem->setFlags(Qt::ItemIsEnabled);

        char size_show[20];
        sprintf(size_show, "%.2f %s", size, FileBs[bs].toStdString().c_str());
        QTableWidgetItem *sizeItem = new QTableWidgetItem(size_show);

        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(Qt::ItemIsEnabled);
        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row,0,fileNameItem);
        filesTable->setItem(row,1,sizeItem);
    }
}

void Window::showDirs(const QDir &directory,const QStringList &dirs, int layers)
{
    for (int i = 0; i < dirs.size(); ++i)
    {
        //self and parent dir not included
        if(strcmp(dirs[i].toStdString().c_str(), ".") && strcmp(dirs[i].toStdString().c_str(), ".."))
        {
            string spaces;
            for(int i = 0; i < layers; ++i) spaces += "-";
            QTableWidgetItem *fileNameItem = new QTableWidgetItem(QString::fromStdString(spaces) + dirs[i]);
            fileNameItem->setFlags(Qt::ItemIsEnabled);

            int row = filesTable->rowCount();
            filesTable->insertRow(row);
            filesTable->setItem(row,0,fileNameItem);

            QFileInfo fileinfo(directory.absoluteFilePath(dirs[i]));
            if(fileinfo.isDir())
            {
                QDir sub_directory = QDir(directory.absoluteFilePath(dirs[i]));
                QStringList sub_dirs  = sub_directory.entryList(QStringList("*"), QDir::Dirs);
                QStringList sub_files = sub_directory.entryList(QStringList("*"), QDir::Files | QDir::NoSymLinks);
                showDirs(sub_directory, sub_dirs, layers + 1);
                showFiles(sub_directory, sub_files, layers + 1);
            }
        }
    }
}

QPushButton *Window::createButton(const QString &text,const char *member)
{
    QPushButton *button = new QPushButton(text);
    connect(button,SIGNAL(clicked()), this, member);
    return button;
}

QComboBox *Window::createComboBox(const QString &text, const bool editable)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->addItem(text);
    comboBox->setEditable(editable);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return comboBox;
}

void Window::initFilesTable()
{
    //create a table height 0 width 3
    filesTable = new QTableWidget(0, 2);
    filesTable->resize(380,800);

    //set table column titles
    QStringList labels;
    labels << tr("Name") << tr("Size");
    filesTable->setHorizontalHeaderLabels(labels);

    filesTable->verticalHeader()->hide();
    filesTable->verticalHeader()->setDefaultSectionSize(10);
    filesTable->setShowGrid(false);

    //set initial column width
    int Table_width = filesTable->width();
    filesTable->setColumnWidth(0, static_cast<int>(0.8  * Table_width));
    filesTable->setColumnWidth(1, static_cast<int>(0.20 * Table_width));
}

void Window::try_connect()
{
    QString ip_addr = ipaddrComboBox->currentText();
    QString port = portComboBox->currentText();
    connect_status = init_connect(ref(client), ip_addr.toStdString().c_str(), port.toInt());
    if(connect_status)
    {
        connectStatusLabel->setText("Connected");
        QMessageBox* msgb = new QMessageBox("", "Connect Success!", QMessageBox::NoIcon, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        msgb->exec();
    }
    else
    {
        QMessageBox* msgb = new QMessageBox("", "Connect Failed!", QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        msgb->exec();
    }
}

void Window::try_send()
{
    if(!connect_status)
    {
        QMessageBox* msgb = new QMessageBox("", "Please Connect First!", QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        msgb->exec();
    }
    else
    {
        QString dir = directoryComboBox->currentText();
        if(dir.length() > 0)
        {
            if(!start_send(client, dir.toStdString().c_str(), 1))
            {
                connectStatusLabel->setText("not connected");
                connect_status = false;
                QMessageBox* msgb = new QMessageBox("", "Connect Interrupted!", QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
                msgb->exec();
            }
        }
        QMessageBox* msgb = new QMessageBox("", "All Files Are Sent", QMessageBox::NoIcon, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        msgb->exec();
    }
}

