#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ini.h"

#include "dialog.h"
#include "editor.h"
#include "sprite.h"
#include "object.h"

#include <QMessageBox>
#include <QMenu>
#include <QVariant>
#include <QFileDialog>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

enum
{
    TAB_TEXTURES,
    TAB_SOUNDS,
    TAB_FUNCTIONS,
    TAB_OBJECTS,
    TAB_ROOMS,
    TAB_COMPILATION
};

// CONSTRUCTOR
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMinimumSize(size());

    ui->listWidget->setIconSize(QSize(128,16));
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(ShowContextMenu(const QPoint&)));

    directory = "C:/";
    ui->tabWidget->setEnabled(false);
}

// DESTRUCTOR
MainWindow::~MainWindow()
{
    delete ui;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// GET ENTRY INFO
bool MainWindow::isFolder(int index)
{
    if (index == -1) return false;
    return ui->listWidget->item(index)->textColor().red() > 0;
}

bool MainWindow::isOpened(int index)
{
    if (index == -1) return false;
    return ui->listWidget->item(index)->textColor().red() == 2;
}

int MainWindow::getLevel(int index)
{
    if (index == -1) return 0;
    return ui->listWidget->item(index)->textColor().blue();
}

// SET ENTRY INFO
void MainWindow::setFolder(int index, bool folder)
{
    QListWidgetItem *item = ui->listWidget->item(index);
    int level = getLevel(index);

    item->setTextColor( QColor(folder, 0, level) );
    if (folder)
        item->setIcon(QIcon("Icons/closed" + QString::number(level) + ".png"));
    else
        item->setIcon(QIcon("Icons/file" + QString::number(level) + ".png"));
}

void MainWindow::setOpened(int index, bool opened)
{
    QListWidgetItem *item = ui->listWidget->item(index);
    int level = getLevel(index);

    item->setTextColor( QColor(1+opened, 0, level) );
    if (opened)
        item->setIcon(QIcon("Icons/opened" + QString::number(level) + ".png"));
    else
        item->setIcon(QIcon("Icons/closed" + QString::number(level) + ".png"));
}

void MainWindow::setLevel(int index, int level)
{
    ui->listWidget->item(index)->setTextColor( QColor(isFolder(index) + isOpened(index), 0, level) );
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

// LOAD ENTRY
void MainWindow::loadEntryList(QString path, int index, int level)
{
    QDir dir(path);
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst);

    for(int k=0; k<2; k++)
    {
        for(int i=list.size()-1; i>=0; i--)
        {
            if (!list.at(i).isFile())
            {
                QFileInfo file = list.at(i);
                bool entry = file.fileName().endsWith(".entry");
                if (k == entry) continue; // Dirs will be first

                QString entryName = file.fileName();
                if (entry) entryName = iniRead(path + entryName + "/entry.ini", "name", "ERROR");
                ui->listWidget->insertItem(index, entryName);

                setLevel(index, level);
                setFolder(index, !entry);
            }
        }
    }
}

// GET FOLDER PATH
QString MainWindow::getFolderPath(int index)
{
    if (index == -1) return directory;

    struct object
    {
        QString name;
        object *link;
    } *list = NULL;

    // find path
    int myLevel = getLevel(index);
    for(int i = ui->listWidget->currentRow()-1; i >= 0; i--)
    {
        if (isFolder(i)) {
            if (isOpened(i)) {
                if (getLevel(i) < myLevel)
                {
                    myLevel--;
                    object *node = new object;
                    node->link = list;
                    list = node;
                    list->name = ui->listWidget->item(i)->text();
                }
            }
        }
    }

    // concatenate
    QString path = directory;
    object *node = list;
    while(node)
    {
        path += node->name + "/";
        list = node;
        node = node->link;
        delete list;
    }
    return path;
}

// GET FILE PATH
QString MainWindow::getFilePath(int index)
{
    if (index == -1) return directory;

    if (isFolder(index))
        return getFolderPath(index) + ui->listWidget->item(index)->text() + "/";
    else
        return getFolderPath(index) + getStringCrop(ui->listWidget->item(index)->text()) + ".entry/";
}

// RIGHT CLICK
void MainWindow::ShowContextMenu(const QPoint& pos)
{
    int index = ui->listWidget->indexAt(pos).row();
    bool check_name = false;
    if (ui->tabWidget->currentIndex() != TAB_FUNCTIONS)
        check_name = true;

    QMenu subMenu;
    subMenu.setTitle("Put in folder");
    subMenu.addAction("Add file");
    subMenu.addAction("Add folder");

    QMenu menu;
    menu.addAction("Create entry");
    menu.addAction("Create group");
    //myMenu.addAction("Duplicate"); // ->setEnabled( isFolder(index) );
    menu.addSeparator();
    menu.addAction("Out of folder")->setEnabled(index != -1);
    menu.addMenu(&subMenu)->setEnabled(index != -1);
    menu.addSeparator();
    menu.addAction("Rename")->setEnabled(index != -1);
    menu.addAction("Delete")->setEnabled(index != -1);

    QAction* selectedItem = menu.exec( ui->listWidget->mapToGlobal(pos) );
    if (!selectedItem) return;

    // DELETE
    if (selectedItem->text() == "Delete")
    {
        if (QMessageBox::question(NULL, "Deletion", "Are you sure?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (!removeDir( getFilePath(index) ))
            {
                message("Unable to delete entry.");
                return;
            }
            delete ui->listWidget->item(index);
        }
        return;
    }

    // CREATE DIALOG WINDOW
    Dialog *dialog = new Dialog(this);
    dialog->check_name = check_name;
    if (selectedItem->text() == "Rename")
        dialog->setString( ui->listWidget->item(index)->text() );
    dialog->exec();
    if (dialogReceiver == "") return;

    // RENAME
    if (selectedItem->text() == "Rename")
    {
        QDir dir( getFolderPath(index) );
        bool result = false;
        if (isFolder(index))
            result = dir.rename( ui->listWidget->item(index)->text(), getStringCrop(dialogReceiver) );
        else
        {
            QString oldName = getStringCrop(ui->listWidget->item(index)->text()) + ".entry";
            QString newName = getStringCrop(dialogReceiver) + ".entry";
            result = dir.rename( oldName, newName ) || oldName == newName;
            if (result)
            {
                iniWrite( dir.path() + "/" + getStringCrop(dialogReceiver) + ".entry/entry.ini", "name", dialogReceiver);
            }
        }

        if (!result)
        {
            message("Unable to rename entry.");
            return;
        }

        ui->listWidget->item(index)->setText( dialogReceiver );
        return;
    }

    // ADD FILE
    if (selectedItem->text() == "Create entry")
    {
        QDir dir( isFolder(index) ? getFilePath(index) : getFolderPath(index) );
        if (!dir.mkdir( getStringCrop(dialogReceiver) + ".entry" ))
        {
            message("Unable to create entry.");
            return;
        }

        iniWrite( dir.path() + "/" + getStringCrop(dialogReceiver) + ".entry/entry.ini", "name", dialogReceiver);
        ui->listWidget->insertItem( index+1, dialogReceiver );

        setLevel(index+1, getLevel(index) + isFolder(index));
        setFolder(index+1, false);
        if (isFolder(index)) setOpened(index, true);
    }

    // ADD FOLDER
    if (selectedItem->text() == "Create group")
    {
        QDir dir( isFolder(index) ? getFilePath(index) : getFolderPath(index) );
        if (!dir.mkdir( getStringCrop(dialogReceiver) ))
        {
            message("Unable to create group.");
            return;
        }
        ui->listWidget->insertItem( index+1, getStringCrop(dialogReceiver) );

        setLevel(index+1, getLevel(index) + isFolder(index));
        setFolder(index+1, true);
        if (isFolder(index)) setOpened(index, true);
    }
}

// DOUBLE CLICK
void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    int index = ui->listWidget->currentRow();

    int level = getLevel(index);
    QString path = getFilePath(index);

    // WHAT TO DO
    if (isFolder(index))
    {
        if (isOpened(index))
        {
            // CLOSE FOLDER
            setOpened(index, false);

            int count = ui->listWidget->count();
            for(int i = index+1; i<count; i++)
            {
                if (getLevel(index+1) > level)
                    delete ui->listWidget->item(index+1);
                else break;
            }
        }
        else
        {
            // OPEN FOLDER
            setOpened(index, true);

            loadEntryList(path, index+1, level+1);
        }
    }
    else
    {
        // WORK WITH FILE
        QString fileName = getFolderPath(index) + getStringCrop(item->text()) + ".entry/";

        int tab = ui->tabWidget->currentIndex();

        if (tab == TAB_TEXTURES)
        {
            Sprite *sprite = new Sprite(this);
            sprite->load( fileName );
            sprite->exec();
        }

        if (tab == TAB_FUNCTIONS)
        {
            fileName += "function.txt";
            Editor *editor = new Editor(this);
            editor->load( fileName );
            editor->exec();
        }

        if (tab == TAB_OBJECTS)
        {
            Object *object = new Object(this);
            object->load( fileName, directory );
            object->exec();
        }

        if (tab == TAB_ROOMS)
        {
            fileName += "room.txt";
            Editor *editor = new Editor(this);
            editor->load( fileName );
            editor->exec();
        }
    }
}

// CREATE
void MainWindow::on_actionCreate_triggered()
{
    // CREATE DIALOG WINDOW
    Dialog *dialog = new Dialog(this);
    dialog->exec();
    if (dialogReceiver == "") return;

    QDir dir(QDir::currentPath() + "/Projects/");
    if (!dir.mkdir( dialogReceiver ))
    {
        message("Failed to create project.");
        return;
    }

    directory = dir.path() + "/" + dialogReceiver + "/";
    dir.setPath(directory);
    dir.mkdir("Textures");
    dir.mkdir("Sounds");
    dir.mkdir("Functions");
    dir.mkdir("Objects");
    dir.mkdir("Rooms");

    ui->lineEditTitle->setText( iniRead(directory + "settings.ini", "title", "title") );
    ui->spinBoxWidth->setValue( iniRead(directory + "settings.ini", "width", 800) );
    ui->spinBoxHeight->setValue( iniRead(directory + "settings.ini", "height", 600) );
    ui->spinBoxFps->setValue( iniRead(directory + "settings.ini", "fps", 60) );
    ui->checkBoxFullscreen->setChecked( iniRead(directory + "settings.ini", "full", 0) );
    ui->checkBoxTextures->setChecked( iniRead(directory + "settings.ini", "textures", 0) );
    ui->checkBoxCursor->setChecked( iniRead(directory + "settings.ini", "cursor", 0) );
    ui->checkBoxBorderless->setChecked( iniRead(directory + "settings.ini", "border", 0) );
    ui->checkBoxResize->setChecked( iniRead(directory + "settings.ini", "resize", 0) );
    ui->checkBoxMinimize->setChecked( iniRead(directory + "settings.ini", "minimize", 0) );

    on_tabWidget_currentChanged( ui->tabWidget->currentIndex() );
    ui->tabWidget->setEnabled(true);
}

// OPEN
void MainWindow::on_actionOpen_triggered()
{
    QString path = QFileDialog::getExistingDirectory(this, "Open project", QDir::currentPath() + "/Projects/",
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (path == "") return;
    directory = path + "/";

    ui->lineEditTitle->setText( iniRead(directory + "settings.ini", "title", "title") );
    ui->spinBoxWidth->setValue( iniRead(directory + "settings.ini", "width", 800) );
    ui->spinBoxHeight->setValue( iniRead(directory + "settings.ini", "height", 600) );
    ui->spinBoxFps->setValue( iniRead(directory + "settings.ini", "fps", 60) );
    ui->checkBoxFullscreen->setChecked( iniRead(directory + "settings.ini", "full", 0) );
    ui->checkBoxTextures->setChecked( iniRead(directory + "settings.ini", "textures", 0) );
    ui->checkBoxCursor->setChecked( iniRead(directory + "settings.ini", "cursor", 0) );
    ui->checkBoxBorderless->setChecked( iniRead(directory + "settings.ini", "border", 0) );
    ui->checkBoxResize->setChecked( iniRead(directory + "settings.ini", "resize", 0) );
    ui->checkBoxMinimize->setChecked( iniRead(directory + "settings.ini", "minimize", 0) );

    QIcon ico( directory + "icon.ico" );
    ui->pushButtonIcon->setIcon( ico );

    on_tabWidget_currentChanged( ui->tabWidget->currentIndex() );
    ui->tabWidget->setEnabled(true);

}

// EXIT
void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    ui->listWidget->clear();

    if (directory.endsWith("Textures/"))  directory.resize( directory.length() - 9 );
    if (directory.endsWith("Sounds/"))    directory.resize( directory.length() - 7 );
    if (directory.endsWith("Functions/")) directory.resize( directory.length() - 10 );
    if (directory.endsWith("Objects/"))   directory.resize( directory.length() - 8 );
    if (directory.endsWith("Rooms/"))     directory.resize( directory.length() - 6 );

    if (index == TAB_TEXTURES)  directory += "Textures/";
    if (index == TAB_SOUNDS)    directory += "Sounds/";
    if (index == TAB_FUNCTIONS) directory += "Functions/";
    if (index == TAB_OBJECTS)   directory += "Objects/";
    if (index == TAB_ROOMS)     directory += "Rooms/";

    if (index == TAB_TEXTURES)  ui->gridLayout_textures->addWidget( ui->listWidget );
    if (index == TAB_SOUNDS)    ui->gridLayout_sounds->addWidget( ui->listWidget );
    if (index == TAB_FUNCTIONS) ui->gridLayout_functions->addWidget( ui->listWidget );
    if (index == TAB_OBJECTS)   ui->gridLayout_objects->addWidget( ui->listWidget );
    if (index == TAB_ROOMS)     ui->gridLayout_rooms->addWidget( ui->listWidget );

    ui->listWidget->setVisible( index != TAB_COMPILATION );

    loadEntryList(directory, 0, 0);
}




void MainWindow::on_lineEditTitle_textEdited(const QString &arg1)
{
    iniWrite(directory + "settings.ini", "title", arg1);
}

void MainWindow::on_spinBoxWidth_valueChanged(int arg1)
{
    iniWrite(directory + "settings.ini", "width", arg1);
}

void MainWindow::on_spinBoxHeight_valueChanged(int arg1)
{
    iniWrite(directory + "settings.ini", "height", arg1);
}

void MainWindow::on_spinBoxFps_valueChanged(int arg1)
{
    iniWrite(directory + "settings.ini", "fps", arg1);
}

void MainWindow::on_checkBoxFullscreen_toggled(bool checked)
{
    iniWrite(directory + "settings.ini", "full", checked);
}

void MainWindow::on_checkBoxTextures_toggled(bool checked)
{
    iniWrite(directory + "settings.ini", "textures", checked);
}

void MainWindow::on_checkBoxCursor_toggled(bool checked)
{
    iniWrite(directory + "settings.ini", "cursor", checked);
}

void MainWindow::on_checkBoxBorderless_toggled(bool checked)
{
    iniWrite(directory + "settings.ini", "border", checked);
}

void MainWindow::on_checkBoxResize_toggled(bool checked)
{
    iniWrite(directory + "settings.ini", "resize", checked);
}

void MainWindow::on_checkBoxMinimize_toggled(bool checked)
{
    iniWrite(directory + "settings.ini", "minimize", checked);
}

void MainWindow::on_pushButtonIcon_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Choose icon", directory, "Text Files (*.ico)");
    if (path == "") return;
    QString str = directory + "icon.ico";
    if (path == str) return;
    remove(str.toLocal8Bit().data());
    QFile::copy(path, str);

    QIcon ico(path);
    ui->pushButtonIcon->setIcon(ico);
}
