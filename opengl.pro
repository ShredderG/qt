#-------------------------------------------------
#
# Project created by QtCreator 2016-09-06T10:01:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opengl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialog.cpp \
    editor.cpp \
    object.cpp \
    sprite.cpp \
    highlighter.cpp \
    compilation.cpp \
    ini.cpp

HEADERS  += mainwindow.h \
    dialog.h \
    editor.h \
    object.h \
    sprite.h \
    highlighter.h \
    ini.h

FORMS    += mainwindow.ui \
    dialog.ui \
    editor.ui \
    object.ui \
    sprite.ui
