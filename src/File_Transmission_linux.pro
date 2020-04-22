#-------------------------------------------------
#
# Project created by QtCreator 2020-04-19T19:07:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = File_Transmission_linux
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
    interface/mainwindow.cpp \
    interface/window.cpp \
    src/Client/Client.cpp \
    src/Client/main.cpp \
    src/file.cpp \
    src/packet_load.cpp \
    src/zip.cpp \

HEADERS += \
    include/Client/Client.h \
    include/base.h \
    include/file.h \
    include/packet_load.h \
    include/zip.h \
    interface/debug/moc_predefs.h \
    interface/mainwindow.h \
    interface/window.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    example/test.test \
    example/e2/e4/e5.txt \
    example/e2/e3.txt \
    example/e1.txt \
    example/e2.txt

LIBS += -lz
