#include "object.h"
#include "ui_object.h"

#include "ini.h"
#include "editor.h"
#include "dialog.h"

#include <QFile>

Object::Object(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Object)
{
    ui->setupUi(this);
    setMinimumSize(size());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    /*
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(ShowContextMenu(const QPoint&)));
    */
}

Object::~Object()
{
    delete ui;
}

enum
{
    DECLARATIONS,
    CONSTRUCTOR,
    DESTRUCTOR,
    STEP,
    DRAW
};

// LOAD
void Object::load(QString fileNameNew, QString directory)
{
    fileName = fileNameNew;
    QString object = iniRead(fileName + "entry.ini", "name", "ERROR");
    ui->groupBox->setTitle( object );
    ui->listWidgetMembers->item(CONSTRUCTOR)->setText( object + "()" );
    ui->listWidgetMembers->item(DESTRUCTOR)->setText( "~" + object + "()" );

    ui->checkBoxSolid->setChecked( iniRead(fileName + "entry.ini", "solid", 0) );
    ui->checkBoxPersistent->setChecked( iniRead(fileName + "entry.ini", "persistent", 0) );
    ui->spinBoxPriority->setValue( iniRead(fileName + "entry.ini", "priority", 0) );

    if ( iniRead(fileName + "entry.ini", "id", 0) == 0 )
    {
        int total = iniRead(directory + "entry.ini", "objects", 0) + 1;
        iniWrite(directory + "entry.ini", "objects", total);
        iniWrite(fileName + "entry.ini", "id", total);
    }
}

// SAVE
void Object::on_pushButton_clicked()
{
    iniWrite(fileName + "entry.ini", "solid", ui->checkBoxSolid->isChecked());
    iniWrite(fileName + "entry.ini", "persistent", ui->checkBoxPersistent->isChecked());
    iniWrite(fileName + "entry.ini", "priority", ui->spinBoxPriority->value());
    deleteLater();
}

void Object::on_listWidgetMembers_itemDoubleClicked(QListWidgetItem *item)
{
    int index = ui->listWidgetMembers->currentRow();

    QString path = fileName;
    switch(index)
    {
    case DECLARATIONS:
        path += "declarations.txt";
        break;
    case CONSTRUCTOR:
        path += "constructor.txt";
        break;
    case DESTRUCTOR:
        path += "destructor.txt";
        break;
    case STEP:
        path += "step.txt";
        break;
    case DRAW:
        path += "draw.txt";
        break;
    }

    Editor *editor = new Editor(this);
    editor->load( path );
    editor->exec();
}
