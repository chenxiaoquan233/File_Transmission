QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    interface/mainwindow.cpp \
    interface/window.cpp \
    src/Client/Client.cpp \
    src/Client/main.cpp \
    src/file.cpp \
    src/packet_load.cpp \
    src/zip.cpp \
    src/zlib/adler32.c \
    src/zlib/compress.c \
    src/zlib/crc32.c \
    src/zlib/deflate.c \
    src/zlib/gzclose.c \
    src/zlib/gzlib.c \
    src/zlib/gzread.c \
    src/zlib/gzwrite.c \
    src/zlib/infback.c \
    src/zlib/inffast.c \
    src/zlib/inflate.c \
    src/zlib/inftrees.c \
    src/zlib/trees.c \
    src/zlib/uncompr.c \
    src/zlib/zutil.c

HEADERS += \
    include/Client/Client.h \
    include/base.h \
    include/file.h \
    include/packet_load.h \
    include/zip.h \
    include/zlib/crc32.h \
    include/zlib/deflate.h \
    include/zlib/gzguts.h \
    include/zlib/inffast.h \
    include/zlib/inffixed.h \
    include/zlib/inflate.h \
    include/zlib/inftrees.h \
    include/zlib/trees.h \
    include/zlib/zconf.h \
    include/zlib/zlib.h \
    include/zlib/zutil.h \
    interface/mainwindow.h \
    interface/window.h

FORMS += \

# Default rules for deployment.
qnx: target.path =  /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
