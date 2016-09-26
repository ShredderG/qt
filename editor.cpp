#include "editor.h"
#include "ui_editor.h"

#include "highlighter.h"

#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>

Editor::Editor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Editor)
{
    ui->setupUi(this);
    setMinimumSize(size());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->pushButtonSave->setFocus();

    highlighter = new Highlighter(this);
    ui->textEdit->setTabStopWidth(28);
    ui->textEdit->setLineWrapMode(QTextEdit::NoWrap);
    highlighter->setDocument( ui->textEdit->document() );
    ui->textEdit->setFocus();
}

Editor::~Editor()
{
    delete highlighter;
    delete ui;
}

bool fileSave(QString fileName, QString text)
{
    QFile f(fileName);
    if (!f.open(QFile::WriteOnly | QFile::Text)) return false;
    QTextStream out(&f);
    out << text;
    f.close();
    return true;
}

QString fileLoad(QString fileName)
{
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly | QFile::Text)) return "";
    QTextStream in(&f);
    QString text = in.readAll();
    f.close();
    return text;
}

void Editor::load(QString fileNameNew)
{
    fileName = fileNameNew;
    QString text = fileLoad(fileNameNew);
    ui->textEdit->setText(text);
}

void Editor::on_pushButtonCancel_clicked()
{
    if (QMessageBox::question(NULL, "Exit", "Exit without saving?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        deleteLater();
}

void Editor::on_pushButtonSave_clicked()
{
    fileSave(fileName, ui->textEdit->toPlainText());
    deleteLater();
}

// INDENT
void Editor::on_pushButtonIndent_clicked()
{
    QString in = ui->textEdit->toPlainText();
    QString out;
    int tabs = 0;

    bool inQuote  = false;
    bool inQuote2 = false;

    unsigned char
            prev = '1',
            curr = '1',
            next = '1';

    for(int i=0; i<in.length(); i++)
    {
        curr = in[i].toLatin1();
        if (i != in.length() - 1) next = in[i+1].toLatin1();
        switch(curr)
        {
        case '\n':
            out += in[i];
            for(int j=0; j<tabs; j++)
                out += '\t';
            break;

        case '-':
        case '+':
        case '/':
        //case '*':
        case '%':
        case '<':
        case '>':
        //case '&':
        case '|':
        case '^':
        case '=':
        case '!':
            if (inQuote || inQuote2)
            {
                out += in[i];
                break;
            }

            if (curr == '-' && next == '>' || prev == '-' && curr == '>')
            {
                out += in[i];
                break;
            }

            if (curr == '!' && next != '=')
            {
                out += in[i];
                break;
            }

            if (curr == '-' && (next == '-' || prev == '-'))
            {
                out += in[i];
                break;
            }

            if (curr == '+' && (next == '+' || prev == '+'))
            {
                out += in[i];
                break;
            }

            if (prev != '\t' && prev != '\n' && prev != ' ')
                if (prev != '-' && prev != '+' && prev != '/' && prev != '*')
                    if (prev != '%' && prev != '<' && prev != '>' && prev != '&')
                        if (prev != '|' && prev != '^' && prev != '=' && prev != '!')
                                out += ' ';

            out += in[i];

            if (next != '\t' && next != '\n' && next != ' ')
                if (next != '-' && next != '+' && next != '/' && next != '*')
                    if (next != '%' && next != '<' && next != '>' && next != '&')
                        if (next != '|' && next != '^' && next != '=' && next != '!')
                                out += ' ';

            break;

        case ',':
        case ';':
            if (inQuote || inQuote2)
            {
                out += in[i];
                break;
            }
            out += in[i];
            if (next != '\t' && next != '\n' && next != ' ')
                out += ' ';
            break;

        case ' ':
            if (out[out.length()-1] != '\t' && out[out.length()-1] != '\n')
                out += in[i];
            break;

        case '\t':
            break;

        case '{':
        case '(':
        case '[':
            out += in[i];
            if (!inQuote2 && !inQuote) tabs++;
            break;

        case '}':
        case ')':
        case ']':
            if (!inQuote2 && !inQuote) tabs--;
            if (out[out.length()-1] == '\t')
                out[out.length()-1] = in[i];
            else
                out += in[i];
            break;

        case '\"':
            if (inQuote2) tabs--;
            else tabs++;
            inQuote2 = !inQuote2;
            out += in[i];
            break;

        case '\'':
            if (!inQuote2)
            {
                if (inQuote) tabs--;
                else tabs++;
                inQuote = !inQuote;
            }
            out += in[i];
            break;

        case '\\':
            out += in[i];
            i++;
            out += in[i];
            break;

        default:
            out += in[i];
        }
        prev = curr;
    }

    if (tabs != 0) QMessageBox::information(NULL, "Oops...", "An error occured.");

    int val = ui->textEdit->verticalScrollBar()->value();
    ui->textEdit->setText(out);
    ui->textEdit->verticalScrollBar()->setValue(val);
}
