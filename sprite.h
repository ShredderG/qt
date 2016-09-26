#ifndef SPRITE_H
#define SPRITE_H

#include <QDialog>

namespace Ui {
class Sprite;
}

class Sprite : public QDialog
{
    Q_OBJECT

public:
    explicit Sprite(QWidget *parent = 0);
    ~Sprite();
    void load(QString);

private slots:
    void on_pushButtonSave_clicked();

    void on_pushButtonEdit_clicked();

    void on_spinBoxWidth_valueChanged(int arg1);

    void on_spinBoxHeight_valueChanged(int arg1);

    void loadSize();
    void setSize();

private:
    Ui::Sprite *ui;
    QString fileName;
    int width;
    int height;
    int xSize;
    int ySize;
};

#endif // SPRITE_H
