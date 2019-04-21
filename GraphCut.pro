#-------------------------------------------------
#
# Project created by QtCreator 2019-04-14T11:10:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GraphCut
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    drawwidget.cpp \
    graphcut.cpp \
    maxflow-v3.04.src/graph.cpp \
    maxflow-v3.04.src/maxflow.cpp

HEADERS += \
        mainwindow.h \
    drawwidget.h \
    graphcut.h \
    maxflow-v3.04.src/block.h \
    maxflow-v3.04.src/graph.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += C:\OpenCV\build_x86\install\include
INCLUDEPATH += C:\OpenCV\build_x86\install\include\opencv
INCLUDEPATH += C:\OpenCV\build_x86\install\include\opencv2

#LIBS += C:\OpenCV\build_x86\install\x86\vc15\lib\opencv_img_hash345d.lib
#LIBS += C:\OpenCV\build_x86\install\x86\vc15\lib\opencv_world345d.lib


LIBS += C:\OpenCV\build_x86\install\x86\vc15\lib\opencv_img_hash345.lib
LIBS += C:\OpenCV\build_x86\install\x86\vc15\lib\opencv_world345.lib

CONFIG += mobility
MOBILITY = 


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
