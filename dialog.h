#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void setString(QString);
    bool check_name;

private slots:
    void on_pushButtonCancel_clicked();

    void on_pushButtonAccept_clicked();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
