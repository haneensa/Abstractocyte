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
    mpstate.cpp \
    statesrelations.cpp \
    mesh.cpp

HEADERS  += mainwindow.h \
    mousepad.h \
    glwidget.h \
    mainopengl.h \
    object.h \
    node.h \
    edge.h \
    graph.h \
    mpstate.h \
    statesrelations.h \
    mesh.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc \

DISTFILES += \
    note.txt
