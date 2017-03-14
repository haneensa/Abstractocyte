#-------------------------------------------------
#
# Project created by QtCreator 2016-11-11T13:58:58
#
#-------------------------------------------------

QT       += core gui opengl xml

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
    abstractionspace.cpp \
    graphmanager.cpp \
    spatialhash.cpp \
    skeleton.cpp \
    skeletonbranch.cpp \
    mesh.cpp \
    openglmanager.cpp \
    datacontainer.cpp \
    glycogen.cpp \
    octree.cpp \
    glycogencluster.cpp \
    glycogenanalysismanager.cpp \
    dbscan.cpp \
    rendervertexdata.cpp \
    trimesh.cpp

HEADERS  += mainwindow.h \
    mousepad.h \
    glwidget.h \
    mainopengl.h \
    object.h \
    node.h \
    edge.h \
    graph.h \
    colors.h \
    abstractionspace.h \
    graphmanager.h \
    spatialhash.h \
    skeleton.h \
    skeletonbranch.h \
    mesh.h \
    ssbo_structs.h \
    glsluniform_structs.h \
    openglmanager.h \
    datacontainer.h \
    glycogen.h \
    octree.h \
    glycogencluster.h \
    glycogenanalysismanager.h \
    dbscan.h \
    rendervertexdata.h \
    glwidget.cpp.autosave \
    trimesh.h \
    trimesh_types.h


FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc \

DISTFILES += \
    note.txt
