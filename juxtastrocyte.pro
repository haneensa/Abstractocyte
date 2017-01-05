#-------------------------------------------------
#
# Project created by QtCreator 2016-11-11T13:58:58
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


CONFIG += c++11

TARGET = juxtastrocyte
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mousepad.cpp \
    glwidget.cpp \
    mainopengl.cpp \
    object.cpp

HEADERS  += mainwindow.h \
    mousepad.h \
    glwidget.h \
    mainopengl.h \
    object.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc \

DISTFILES += \
    note.txt
