#ifndef INI
#define INI

#include <QSettings>

QString iniRead(QString filename, QString name, QString value);
float iniRead(QString filename, QString name, float value);
void iniWrite(QString filename, QString name, QString value);
void iniWrite(QString filename, QString name, float value);

// DELETE FIRECTORY
bool removeDir(const QString &dirName);

// FCUKING MESSAGE
void message(QString text);

// STRING WORK
QString getStringCrop(QString full);

// ENTRY LIST
struct entryList
{
    entryList *link;
    QString path;
    entryList(entryList **entryTop, QString pathNew)
    {
        link = *entryTop;
        (*entryTop) = this;
        path = pathNew;
    }
    ~entryList()
    {
        if (link)
            delete link;
    }
};

#endif // INI

