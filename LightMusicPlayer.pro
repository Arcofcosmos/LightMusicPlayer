#-------------------------------------------------
#
# Project created by QtCreator 2021-07-19T15:18:22
#
#-------------------------------------------------

QT       += core gui sql
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LightMusicPlayer
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
        mainwidget.cpp \
    musiclist.cpp \
    musiclistwidget.cpp \
    musiclistwidgetdialog.cpp \
    lyricwidget.cpp \
    music.cpp \
    tcpthread.cpp

HEADERS += \
        mainwidget.h \
    musiclist.h \
    musiclistwidget.h \
    musiclistwidgetdialog.h \
    lyricwidget.h \
    myqss.h \
    music.h \
    tcpthread.h

FORMS += \
        mainwidget.ui \
    musiclistwidgetdialog.ui \
    lyricwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

DISTFILES += \
    logo.rc

RC_FILE += logo.rc
