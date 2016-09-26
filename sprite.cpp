#include "sprite.h"
#include "ui_sprite.h"

#include "ini.h"
#include <QFile.h>
#include <QProcess.h>

Sprite::Sprite(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Sprite)
{
    ui->setupUi(this);
    setMinimumSize(size());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

Sprite::~Sprite()
{
    delete ui;
}

void Sprite::loadSize()
{
    QString file = fileName + "texture.bmp";
    FILE *f = fopen( file.toLocal8Bit().data(), "rb" );

    // ширина
    width = 0;
    height = 0;
    if (f)
    {
        fseek(f, 18, SEEK_SET);
        fread(&width, 4, 1, f);
        // высота
        fseek(f, 22, SEEK_SET);
        fread(&height, 4, 1, f);
        fclose(f);
    }
    else
    {
        QFile fff(file);
        fff.open(QIODevice::WriteOnly);
        fff.close();
    }

    ui->labelWidth ->setText(QString::number(width));
    ui->labelHeight->setText(QString::number(height));

    ui->labelSprite->setPixmap( QPixmap(file) );

    if (width != 0 && height != 0)
    {
        setMaximumWidth( minimumWidth() - 256 + width );
        setMaximumHeight( minimumHeight() - 256 + height );
    }
}

void Sprite::setSize()
{
    if (xSize != 0 && ySize != 0)
        ui->labelCount->setText( QString::number( (width / xSize) * (height / ySize) ) );
    else
        ui->labelCount->setText( "0" );
}

void Sprite::load(QString fileNameNew)
{
    fileName = fileNameNew;
    for(int i=0; i<fileName.length(); i++)
        if (fileName[i] == '/') fileName[i] = '\\';

    ui->groupBox_3->setTitle( iniRead(fileName + "entry.ini", "name", "ERROR") );

    xSize = iniRead(fileName + "entry.ini", "xSize", 0);
    ySize = iniRead(fileName + "entry.ini", "ySize", 0);

    loadSize();
    ui->spinBoxWidth->setValue(xSize);
    ui->spinBoxHeight->setValue(ySize);
    setSize();
}

// SAVE BUTTON
void Sprite::on_pushButtonSave_clicked()
{
    iniWrite(fileName + "entry.ini", "xSize", xSize);
    iniWrite(fileName + "entry.ini", "ySize", ySize);
    deleteLater();
}

// EDIT BUTTON
void Sprite::on_pushButtonEdit_clicked()
{
    QProcess editor;
    editor.execute("mspaint.exe \"" + fileName + "texture.bmp\"");
    editor.waitForFinished();

    loadSize();
    setSize();

    iniWrite(fileName + "entry.ini", "width", width);
    iniWrite(fileName + "entry.ini", "height", height);
}

void Sprite::on_spinBoxWidth_valueChanged(int arg1)
{
    xSize = arg1;
    if (xSize > width)
    {
        xSize = width;
        ui->spinBoxWidth->setValue(xSize);
    }
    setSize();
}

void Sprite::on_spinBoxHeight_valueChanged(int arg1)
{
    ySize = arg1;
    if (ySize > height)
    {
        ySize = height;
        ui->spinBoxHeight->setValue(ySize);
    }
    setSize();
}
