#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "ini.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString dialogReceiver;

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void ShowContextMenu(const QPoint& pos);
    QString getFilePath(int index);
    QString getFolderPath(int index);
    bool isFolder(int index);
    bool isOpened(int index);
    int getLevel(int index);
    void setFolder(int index, bool folder);
    void setOpened(int index, bool opened);
    void setLevel(int index, int level);
    void loadEntryList(QString path, int index, int level);

    void on_actionExit_triggered();
    void on_actionOpen_triggered();
    void on_actionCreate_triggered();

    void on_tabWidget_currentChanged(int index);

    void on_pushButtonRun_clicked();
    void getEntries(entryList **node, QString dirName);

    bool compileSprites();
    bool compileFunctions();
    bool compileObjects();
    bool compileSounds();
    bool compileDefines();
    bool compileRooms();

    void on_lineEditTitle_textEdited(const QString &arg1);

    void on_spinBoxWidth_valueChanged(int arg1);

    void on_spinBoxHeight_valueChanged(int arg1);

    void on_spinBoxFps_valueChanged(int arg1);

    void on_checkBoxMinimize_toggled(bool checked);

    void on_checkBoxFullscreen_toggled(bool checked);

    void on_checkBoxTextures_toggled(bool checked);

    void on_checkBoxCursor_toggled(bool checked);

    void on_checkBoxBorderless_toggled(bool checked);

    void on_checkBoxResize_toggled(bool checked);

    void on_pushButtonIcon_clicked();

private:
    Ui::MainWindow *ui;
    QString directory;
};

#endif // MAINWINDOW_H
