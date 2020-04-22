#ifndef WINDOW_H
#define WINDOW_H

#include <QDialog>
#include "../include/Client/Client.h"
class QComboBox;
class QDir;
class QLabel;
class QPushButton;
class QTableWidget;

class Window:public QDialog
{
Q_OBJECT
public:
Window(QWidget *parent = nullptr);
private slots:
    void browse();
    void find();
    void try_connect();
    void try_send();
private:
    bool connect_status;
    Client* client;
    QComboBox *ipaddrComboBox;
    QComboBox *portComboBox;
    QComboBox *directoryComboBox;
    QLabel *ipaddrLabel;
    QLabel *portLabel;
    QLabel *directoryLabel;
    QLabel *connectStatusLabel;
    QPushButton *browseButton;
    QPushButton *sendButton;
    QPushButton *connectButton;
    QTableWidget *filesTable;
    QStringList FileBs = {"B", "KB", "MB", "GB", "TB"};

    void initFilesTable();
    void showDirs(const QDir &directory,const QStringList &dirs, int layers);
    void showFiles(const QDir &directory,const QStringList &files, int layers);

    QStringList findFiles(const QDir &directory,const QStringList &files, const QString &text);
    QPushButton *createButton(const QString &text, const char *member);
    QComboBox *createComboBox(const QString &text, const bool editable);
};

#endif
