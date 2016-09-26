#ifndef OBJECT_H
#define OBJECT_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class Object;
}

class Object : public QDialog
{
    Q_OBJECT

public:
    explicit Object(QWidget *parent = 0);
    ~Object();
    void load(QString fileNameNew, QString directory);

private slots:
    void on_pushButton_clicked();

    void on_listWidgetMembers_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::Object *ui;
    QString fileName;
};

#endif // OBJECT_H
