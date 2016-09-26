#include <QSettings>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>

// FCUKING MESSAGE
void message(QString text)
{
    QMessageBox::information(NULL, "Message", text);
}

QString iniRead(QString filename, QString name, QString value)
{
    QFile f(filename);
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return value;
    QTextStream in(&f);

    while(!in.atEnd())
    {
        QString line = in.readLine();
        int ravno = line.indexOf('=');

        if (line.left( ravno ) == name)
        {
            f.close();
            return line.mid( ravno+1, line.length()-ravno-1);
        }
    }

    f.close();
    return value;
}

float iniRead(QString filename, QString name, float value)
{
    return iniRead(filename, name, QString::number(value)).toFloat();
}

void iniWrite(QString filename, QString name, QString value)
{
    QFile f(filename);
    if (f.exists()) f.rename("file.tmp");
    QFile f2(filename);
    if (!f2.open(QFile::WriteOnly | QFile::Text)) return;
    QTextStream in(&f);
    QTextStream out(&f2);

    bool found = false;
    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        while(!in.atEnd())
        {
            QString line = in.readLine();
            int ravno = line.indexOf('=');

            if (line.left( ravno ) == name)
            {
                out << line.left( ravno ) << "=" << value << endl;
                found = true;
            }
            else out << line << endl;
        }
    }
    if (!found)
        out << name << "=" << value << endl;

    f.close();
    f.remove();
    f2.close();
}

void iniWrite(QString filename, QString name, float value)
{
    iniWrite(filename, name, QString::number(value));
}

// DELETE FIRECTORY
bool removeDir(const QString &dirName)
{
    bool result = true;

    QDir dir(dirName);
    if (dir.exists(dirName))
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if (info.isDir())
                result = removeDir(info.absoluteFilePath());
            else
                result = QFile::remove(info.absoluteFilePath());
            if (!result) return result;
        }
        result = dir.rmdir(dirName);
    }

    return result;
}

// STRING WORK
QString getStringCrop(QString full)
{
    QString crop = "";
    for(int i=0; i<full.length(); i++)
    {
        QChar c = full[i];
        if (c != '/')
            if (c != '\\')
                if (c != ':')
                    if (c != '*')
                        if (c != '?')
                            if (c != '\"')
                                if (c != '<')
                                    if (c != '>')
                                        if (c != '|')
                                            crop += c;
    }
    return crop;
}
