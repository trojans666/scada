#-------------------------------------------------
#
# Project created by QtCreator 2021-01-23T21:59:20
#
#-------------------------------------------------
include(scada/scada.pri)

QT       -= core gui

TARGET = main
TEMPLATE = app

OBJECTS_DIR = $$PWD/../pro/obj



LIBS += -L$$PWD/output -lscada

SOURCES += \
    called/main.cpp


