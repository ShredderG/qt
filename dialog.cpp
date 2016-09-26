#include "dialog.h"
#include "ui_dialog.h"
#include "mainwindow.h"
#include <QTimer>
#include <QMessageBox>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setMinimumSize(size());
    setMaximumHeight(height());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QTimer::singleShot(0, ui->lineEdit, SLOT(setFocus()));
    ui->pushButtonAccept->setFocus();
    ((MainWindow*)parent)->dialogReceiver = "";

    check_name = false;
}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::setString(QString text)
{
    ui->lineEdit->setText(text);
}

// CANCEL
void Dialog::on_pushButtonCancel_clicked()
{
    deleteLater();
}

enum
{
    TAB_SPRITES,
    TAB_FUNCTIONS,
    TAB_OBJECTS,
    TAB_COMPILATION
};

// ACCEPT
void Dialog::on_pushButtonAccept_clicked()
{
    QString answer = ui->lineEdit->text();

    if (check_name)
    {
        bool bad = false;
        if (answer == "") bad = true;
        if (answer[0].isDigit()) bad = true;
        for(int i=0; i<answer.length(); i++)
        {
            if (!answer[i].isLetterOrNumber() && answer[i] != '_')
            {
                bad = true;
                break;
            }

        }
        if (bad)
        {
            QMessageBox::information(NULL, "Oops...", "Only letters, digits and \'_\' are allowed.");
            return;
        }
    }

    ((MainWindow*)parent())->dialogReceiver = answer;
    deleteLater();
}
