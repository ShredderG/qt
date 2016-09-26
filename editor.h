#ifndef EDITOR_H
#define EDITOR_H

#include <QDialog>
#include "highlighter.h"

namespace Ui {
class Editor;
}

class Editor : public QDialog
{
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = 0);
    ~Editor();
    void load(QString);

private slots:
    void on_pushButtonCancel_clicked();

    void on_pushButtonSave_clicked();

    void on_pushButtonIndent_clicked();

private:
    Ui::Editor *ui;
    QString fileName;
    Highlighter *highlighter;
};

#endif // EDITOR_H
