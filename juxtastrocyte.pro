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
    object.cpp \
    node.cpp \
    edge.cpp \
    graph.cpp \
    mesh.cpp \
    abstractionspace.cpp \
    graphmanager.cpp \
    spatialhash.cpp \
    intervaltree.cpp

HEADERS  += mainwindow.h \
    mousepad.h \
    glwidget.h \
    mainopengl.h \
    object.h \
    node.h \
    edge.h \
    graph.h \
    mesh.h \
    colors.h \
    abstractionspace.h \
    graphmanager.h \
    spatialhash.h \
    intervaltree.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc \

DISTFILES += \
    note.txt
