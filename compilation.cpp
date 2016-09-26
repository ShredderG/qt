#include "mainwindow.h"

#include "ini.h"

#include <QMessageBox>
#include <QMenu>
#include <QVariant>
#include <QFileDialog>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

// COMPILATION
void MainWindow::on_pushButtonRun_clicked()
{
    removeDir(QDir::currentPath() + "/release/files");
    QDir dir("release");
    dir.mkdir("files");

    if (!compileSprites())   return;
    if (!compileFunctions()) return;
    if (!compileObjects())   return;
    if (!compileSounds())    return;
    if (!compileDefines())   return;
    if (!compileRooms())     return;

    bool ico = false;
    QFile icon(directory + "icon.ico");
    if (icon.exists())
    {
        QFile::copy(directory + "icon.ico", "release/files/icon.ico");
        ico = true;
    }

    QString path[3];
    path[0] = "C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\msbuild.exe";
    path[1] = "C:\\Windows\\Microsoft.NET\\Framework\\v3.5\\msbuild.exe";
    path[2] = "C:\\Windows\\Microsoft.NET\\Framework\\v2.0.50727\\msbuild.exe";

    bool build = false;
    for(int i=0; i<3; i++)
    {
        QFile f(path[i]);
        if (f.exists())
        {
            remove("errors.txt");
            QProcess msbuild;
            if (ico)
                msbuild.execute(path[i] + " \"project.proj\" /t:Rebuild /p:icon=true /p:Configuration=Release /flp:errorsonly;LogFile=release/errors.txt");
            else
                msbuild.execute(path[i] + " \"project.proj\" /t:Rebuild /p:icon=false /p:Configuration=Release /flp:errorsonly;LogFile=release/errors.txt");
            build = true;
            break;
        }
    }

    if (!build)
    {
        message("Cannot find msbuild.exe.");
        return;
    }

    QProcess project;
    project.execute("x64\\release\\project.exe");
    FILE *f = fopen("release/errors.txt", "r");
    if (f)
    {
        char c = 'a';
        fscanf(f, "%c", &c);
        if (c != 'a')
            project.execute("notepad release/errors.txt");
        fclose(f);
    }
}

QString fileLoad(QString fileName);

// GET ENTRIES
void MainWindow::getEntries(entryList **node, QString dirName)
{
    QDir dir(dirName);
    if (dir.exists(dirName))
    {
        if (dirName.endsWith(".entry"))
            new entryList(node, dirName);
        else
        {
            Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
            {
                if (info.isDir())
                    getEntries(node, info.absoluteFilePath());
            }
        }
    }
}

bool MainWindow::compileSprites()
{
    QFile f("release\\files\\textures.cpp");
    if (!f.open(QFile::WriteOnly | QFile::Text))
    {
        message("Cannot open \"textures.cpp\".");
        return false;
    }
    QTextStream out(&f);

    entryList *node = NULL;
    getEntries(&node, directory + "Textures/");
    entryList *deleteNode = node;

    if (node)
    {
        if (!ui->checkBoxTextures->isChecked())
        {
            out << "uchar GM_data[] =" << endl << "{" << endl << "\t";
            int sdvig = 0;
            while(node)
            {
                QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
                int width  = iniRead(node->path + "/entry.ini", "width",  0);
                int height = iniRead(node->path + "/entry.ini", "height", 0);

                if (name == "ERROR" || name == "")
                {
                    message("Cannot get texture name.");
                    f.close();
                    delete deleteNode;
                    return false;
                }

                if (width <= 0 || height <= 0)
                {
                    message("Cannot get \"" + name + "\" texture data.");
                    f.close();
                    delete deleteNode;
                    return false;
                }

                QString fileName = node->path + "/texture.bmp";
                FILE *texture = fopen( fileName.toLocal8Bit().data(), "rb" );
                fseek(texture, 54, SEEK_SET);

                for(uint i=0, length=width*height; i<length; i++)
                {
                    unsigned char c[3];
                    fread(&c, 3, 1, texture);

                    for(int j=2; j>=0; j--)
                    {
                        if (c[j] < 10) out << " ";
                        if (c[j] < 100) out << " ";
                        out << (int)c[j];
                        if (node->link != NULL || j != 0 || i != length-1)
                            out << ", ";
                    }

                    if (++sdvig == 4)
                    {
                        if (node->link != NULL || i != length-1)
                            out << endl << "\t";
                        sdvig = 0;
                    }
                }

                fclose(texture);
                node = node->link;
            }
            out << endl << "};" << endl;
        }

        out << endl << "GM_texture" << endl;

        // GM_texture
        node = deleteNode;
        int textureData = 0;
        while(node)
        {
            QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
            int width  = iniRead(node->path + "/entry.ini", "width",  0);
            int height = iniRead(node->path + "/entry.ini", "height", 0);
            int xSize  = iniRead(node->path + "/entry.ini", "xSize",  0);
            int ySize  = iniRead(node->path + "/entry.ini", "ySize",  0);

            if (name == "ERROR" || name == "")
            {
                message("Cannot get texture name.");
                f.close();
                delete deleteNode;
                return false;
            }

            if (width <= 0 || height <= 0)
            {
                message("Cannot get \"" + name + "\" texture data.");
                f.close();
                delete deleteNode;
                return false;
            }

            if (xSize <= 0 || ySize <= 0 || xSize > width || ySize > height)
            {
                xSize = width;
                ySize = height;
            }

            out << "\t" << name << "(";
            out << QString::number(width)  << ", ";
            out << QString::number(height) << ", ";
            out << QString::number(xSize)  << ", ";
            out << QString::number(ySize)  << ", ";
            if (!ui->checkBoxTextures->isChecked())
                out << "&GM_data[" << QString::number(textureData) << "])";
            else
                out << "\"Resources\\Textures\\" << name << ".bmp\")";

            if (node->link)
                out << "," << endl;
            else
                out << ";" << endl;

            textureData += width * height * 3;

            node = node->link;
        }
    }

    // GM_loadTextures
    out << endl;
    out << "void GM_loadTextures()" << endl;
    out << "{" << endl;
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        out << "\t" << name << ".load();" << endl;
        node = node->link;
    }
    out << "}" << endl;

    // GM_unloadTextures
    out << endl;
    out << "void GM_unloadTextures()" << endl;
    out << "{" << endl;
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        out << "\t" << name << ".unload();" << endl;
        node = node->link;
    }
    out << "}" << endl;

    if (deleteNode) delete deleteNode;
    f.close();
    return true;
}




bool MainWindow::compileFunctions()
{
    QFile f("release\\files\\functions.h");
    if (!f.open(QFile::WriteOnly | QFile::Text))
    {
        message("Cannot open \"functions.h\".");
        return false;
    }
    QTextStream out(&f);

    entryList *node = NULL;
    getEntries(&node, directory + "Functions/");
    entryList *deleteNode = node;

    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        if (name == "ERROR" || name == "")
        {
            message("Cannot get function name.");
            f.close();
            delete deleteNode;
            return false;
        }

        out << name << ";" << endl;
        node = node->link;
    }

    QFile f2("release\\files\\functions.cpp");
    if (!f2.open(QFile::WriteOnly | QFile::Text))
    {
        message("Cannot open \"functions.cpp\".");
        return false;
    }
    QTextStream out2(&f2);

    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");

        out2 << name << endl << "{" << endl;
        out2 << fileLoad( node->path + "/function.txt" );
        out2 << endl << "}" << endl << endl;
        node = node->link;
    }

    if (deleteNode) delete deleteNode;
    f.close();
    f2.close();

    return true;
}




bool MainWindow::compileObjects()
{
    QFile f("release\\files\\objects.cpp");
    if (!f.open(QFile::WriteOnly | QFile::Text))
    {
        message("Cannot open \"objects.cpp\".");
        return false;
    }
    QTextStream out(&f);

    entryList *node = NULL;
    getEntries(&node, directory + "Objects/");
    entryList *deleteNode = node;

    // #define GM_OBJECT_ID_object 1
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        int id = iniRead(node->path + "/entry.ini", "id", 0);
        if (name == "ERROR" || name == "")
        {
            message("Cannot get object name.");
            f.close();
            delete deleteNode;
            return false;
        }
        if (id == 0)
        {
            message("Cannot get \"" + name + "\" id.");
            f.close();
            delete deleteNode;
            return false;
        }

        out << "#define GM_OBJECT_ID_" << name << " " << QString::number(id) << endl;
        node = node->link;
    }
    out << endl;

    /*
    // #define GM_OBJECT_NAME_1 object
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        int id = iniRead(node->path + "/entry.ini", "id", 0);

        out << "#define GM_OBJECT_NAME_" << id << " " << name << endl;
        node = node->link;
    }
    out << endl;


    out << "uint GM_objects[] =" << endl;
    out << "{" << endl;
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        out << "\t" << "GM_OBJECT_ID_" << name << "," << endl;
        node = node->link;
    }
    out << "\t" << "0" << endl;
    out << "};" << endl;
    out << endl;

    out << "string GM_objects_name[] =" << endl;
    out << "{" << endl;
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        out << "\t" << "\"" << name << "\"," << endl;
        node = node->link;
    }
    out << "\t" << "\"NO_OBJECT\"" << endl;
    out << "};" << endl;
    out << endl;
    */

    // #include "object.h"
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        out << "#include \"o_" << name << ".h\"" << endl;
        node = node->link;
    }
    out << endl;

    // objectCreate()
    out << "GM_object *objectCreate(float x, float y, float z, GM_object *GM_ptr)" << endl << "{" << endl;
    out << "\t" << "GM_object *GM_new = NULL;" << endl;

    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        out << "\t" << "if (GM_ptr == " << name << ") GM_new = new GM_OBJECT_" << name << "(x,y,z);" << endl;
        node = node->link;
    }
    out << "\t" << "//if (GM_ptr < (GM_object*)0x0000FFFF) return NULL;" << endl;
    out << "\t" << "GM_new->GM_insert(GM_ptr);" << endl;
    out << "\t" << "return GM_new;" << endl;
    out << "}" << endl << endl;

    // #include "object.cpp"
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        out << "#include \"o_" << name << ".cpp\"" << endl;
        node = node->link;
    }

    // object.h
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        QString object = "GM_OBJECT_" + name;
        QFile f2("release\\files\\o_" + name + ".h");
        if (!f2.open(QFile::WriteOnly | QFile::Text))
        {
            message("Cannot open \"" + name + ".h\".");
            f.close();
            delete deleteNode;
            return false;
        }
        QTextStream out2(&f2);

        out2 << "struct " << object << " : public GM_object" << endl;
        out2 << "{" << endl;

        out2 << fileLoad(node->path + "/declarations.txt") << endl << endl;
        out2 << "\t" << "static int GM_count;" << endl;
        out2 << "\t" << object << "(float GM_x, float GM_y, float GM_z);" << endl;
        out2 << "\t~" << object << "();" << endl;
        out2 << "\tvoid destroy();" << endl;
        out2 << "\tvoid GM_step();" << endl;
        out2 << "\tvoid GM_draw();" << endl;
        out2 << "\tuint GM_id();" << endl;

        out2 << "} *" << name << " = (" << object << "*) GM_OBJECT_ID_" << name << ";" << endl;

        out2 << "\t" << "int " << object << "::GM_count;" << endl;

        f2.close();
        node = node->link;
    }

    // object.cpp
    node = deleteNode;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        QString object = "GM_OBJECT_" + name;
        QFile f2("release\\files\\o_" + name + ".cpp");
        if (!f2.open(QFile::WriteOnly | QFile::Text))
        {
            message("Cannot open \"" + name + ".cpp\".");
            f.close();
            delete deleteNode;
            return false;
        }
        QTextStream out2(&f2);

        out2 << object << "::" << object << "(float GM_x, float GM_y, float GM_z)" << endl;
        out2 << "{" << endl;
        out2 << "GM_count++;" << endl;
        out2 << name + " = this;" << endl;
        out2 << "priority = " << (int)iniRead(node->path + "/entry.ini", "priority", 0) << ";" << endl;
        //out2 << "depth = " << (int)iniRead(node->path + "/entry.ini", "depth", 0) << ";" << endl;
        out2 << "solid = " << (iniRead(node->path + "/entry.ini", "solid", 0) ? "true" : "false") << ";" << endl;
        out2 << "persistent = " << (iniRead(node->path + "/entry.ini", "persistent", 0) ? "true" : "false") << ";" << endl;
        out2 << "x = GM_x;" << endl;
        out2 << "y = GM_y;" << endl;
        out2 << "z = GM_z;" << endl;
        out2 << fileLoad(node->path + "/constructor.txt") << endl;
        out2 << "}" << endl << endl;

        out2 << object << "::~" << object << "()" << endl;
        out2 << "{" << endl;
        out2 << "if (" + name + " == this)" << endl;
        out2 << "{" << endl;
        out2 << "\tif (GM_left)  if (GM_left->GM_id()  == GM_id()) " + name + " = (" + object + "*) GM_left;"  << endl;
        out2 << "\tif (GM_right) if (GM_right->GM_id() == GM_id()) " + name + " = (" + object + "*) GM_right;" << endl;
        out2 << "\tif (" + name + " == this) " + name + " = (" + object + "*) GM_id();" << endl;
        out2 << "}" << endl;
        out2 << "}" << endl;
        out2 << endl;

        out2 << "void " << object << "::destroy()" << endl;
        out2 << "{" << endl;
        out2 << "if (!GM_active) return;" << endl;
        out2 << "GM_count--;" << endl;
        out2 << "GM_active = false;" << endl;
        out2 << fileLoad(node->path + "/destructor.txt") << endl;
        out2 << "}" << endl;
        out2 << endl;

        out2 << "void " << object << "::GM_step()" << endl;
        out2 << "{" << endl;
        out2 << fileLoad(node->path + "/step.txt") << endl;
        out2 << "}" << endl;
        out2 << endl;

        out2 << "void " << object << "::GM_draw()" << endl;
        out2 << "{" << endl;
        out2 << fileLoad(node->path + "/draw.txt") << endl;
        out2 << "}" << endl;
        out2 << endl;

        out2 << "uint " << object << "::GM_id()" << endl;
        out2 << "{" << endl;
        out2 << "return " << "GM_OBJECT_ID_" + name + ";" << endl;
        out2 << "}";

        f2.close();
        node = node->link;
    }

    if (deleteNode) delete deleteNode;
    f.close();

    return true;
}




bool MainWindow::compileSounds()
{
     return true;
}




bool MainWindow::compileDefines()
{
    QFile f("release\\files\\defines.cpp");
    if (!f.open(QFile::WriteOnly | QFile::Text))
    {
        message("Cannot open \"defines.cpp\".");
        return false;
    }
    QTextStream out(&f);

    out << "typedef unsigned int   uint;" << endl;
    out << "typedef unsigned char  uchar;" << endl;
    out << "typedef unsigned short ushort;" << endl;
    out << "typedef unsigned int   uint;" << endl;

    out << "#define GM_TITLE    \"" << ui->lineEditTitle->text() << "\"" << endl;
    out << "#define GM_WIDTH    " << ui->spinBoxWidth->value() << endl;
    out << "#define GM_HEIGHT   " << ui->spinBoxHeight->value() << endl;
    out << "#define GM_FPS      " << ui->spinBoxFps->value() << endl;

    out << "#define GM_FULL     " << (ui->checkBoxFullscreen->isChecked() ? "true" : "false") << endl;
    out << "#define GM_MAXIMIZE " << (ui->checkBoxResize->isChecked()     ? "true" : "false") << endl;
    out << "#define GM_MINIMIZE " << (ui->checkBoxMinimize->isChecked()   ? "true" : "false") << endl;
    out << "#define GM_MOUSE    " << (ui->checkBoxCursor->isChecked()     ? "true" : "false") << endl;
    out << "#define GM_NOBORDER " << (ui->checkBoxBorderless->isChecked() ? "true" : "false") << endl;

    out << "uint fps = GM_FPS;" << endl;
    out << "bool GM_game = true;";

    f.close();
    return true;
}




bool MainWindow::compileRooms()
{
    QFile f("release\\files\\rooms.cpp");
    if (!f.open(QFile::WriteOnly | QFile::Text))
    {
        message("Cannot open \"rooms.cpp\".");
        return false;
    }
    QTextStream out(&f);

    entryList *node = NULL;
    getEntries(&node, directory + "Rooms/");
    entryList *deleteNode = node;

    // GM_room room;
    node = deleteNode;
    while(node)
    {
        QString filename = node->path + "/room.ini";
        //int count = iniRead(filename, "count", 0);
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");
        if (name == "ERROR" || name == "")
        {
            message("Cannot get room name.");
            f.close();
            delete deleteNode;
            return false;
        }

        out << "void GM_ROOM_" << name << "()" << endl;
        out << "{" << endl;
        /*for(int i=0; i<count; i++)
        {
            QString number = QString::number(count);
            float x = iniRead(filename, "x" + number, 0);
            float y = iniRead(filename, "y" + number, 0);
            float z = iniRead(filename, "z" + number, 0);
            float o = iniRead(filename, "o" + number, 0);
            out << "\t" << "objectCreate(" << x << ", " << y << ", " << z << ", GM_OBJECT_NAME_" << o << ");" << endl;
        }*/

        out << fileLoad(node->path + "/room.txt") << endl;

        out << "}" << endl;
        out << endl;

        node = node->link;
    }

    // GM_room room;
    node = deleteNode;
    if (node) out << "GM_room" << endl;
    while(node)
    {
        QString name = iniRead(node->path + "/entry.ini", "name", "ERROR");

        out << "\t" << name << "(GM_ROOM_" << name << ")" << (node->link ? "," : ";") << endl;

        node = node->link;
    }

    if (deleteNode) delete deleteNode;
    f.close();
    return true;
}
