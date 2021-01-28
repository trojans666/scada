#-------------------------------------------------
#
# Project created by QtCreator 2021-01-23T21:59:20
#
#-------------------------------------------------


QT       -= core gui

TARGET = main
TEMPLATE = app

OBJECTS_DIR = $$PWD/../pro/main

DESTDIR = $$PWD/../output

INCLUDEPATH += $$PWD/../src/core \
            $$PWD/../src/event \
            $$PWD/../src/excep \
            $$PWD/../src/subsys \
            $$PWD/../src/xml

unix {
    target.path = $$DESTDIR
    INSTALLS += target
}

LIBS += -L$$PWD/../output -lscada

SOURCES += \
    main.cpp




