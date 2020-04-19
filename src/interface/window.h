#ifndef WINDOW_H
#define WINDOW_H

#include <QDialog>
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
private:
    QComboBox *fileComboBox;
    QComboBox *textComboBox;
    QComboBox *directoryComboBox;
    QLabel *directoryLabel;
    QPushButton *browseButton;
    QPushButton *sendButton;
    QTableWidget *filesTable;
    QStringList FileBs = {"B", "KB", "MB", "GB", "TB"};

    void initFilesTable();
    void showDirs(const QDir &directory,const QStringList &dirs);
    void showFiles(const QDir &directory,const QStringList &files);

    QStringList findFiles(const QDir &directory,const QStringList &files, const QString &text);
    QPushButton *createButton(const QString &text, const char *member);
    QComboBox *createComboBox(const QString &text, const bool editable);
};

#endif
