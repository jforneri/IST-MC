#-------------------------------------------------
#
# Project created by QtCreator 2012-01-18T17:08:31
#
#-------------------------------------------------

QT       += core gui

TARGET = montecarlo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    filesdialog.cpp \
    filebrowser.cpp \
    dialog_setup.cpp \
    qcustomplot.cpp \
    dialog_import.cpp \
    radhard_dialog.cpp \
    licensedialog.cpp

HEADERS  += mainwindow.h \
    filesdialog.h \
    filebrowser.h \
    first_header.h \
    dialog_setup.h \
    generatori.h \
    qcustomplot.h \
    dialog_import.h \
    MyLibraryWrapper.h \
    radhard_dialog.h \
    licensedialog.h

FORMS    += mainwindow.ui \
    filesdialog.ui \
    filebrowser.ui \
    dialog_setup.ui \
    dialog_import.ui \
    radhard_dialog.ui \
    licensedialog.ui
