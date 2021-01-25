TARGET = mod_ui_qtstart
TEMPLATE = lib

CONFIG += plugin

QT += gui core

# CONFIG += release #debug_and_release debug

DESTDIR = $$PWD/../../output/modules


unix {
    target.path = $$DESTDIR
    INSTALLS += target
}

#LIBS += -L$$PWD/../../output -lscada

OBJECTS_DIR = $$PWD/../../pro/obj
MOC_DIR = $$PWD/../../pro/obj

INCLUDEPATH += /usr/loca/Trolltech/Qt4.8.6/include

INCLUDEPATH += $$PWD/../../src/core \
            $$PWD/../../src/event \
            $$PWD/../../src/excep \
            $$PWD/../../src/subsys \
            $$PWD/../../src/xml

HEADERS += \
    QTStarter/ui_start.h

SOURCES += \
    QTStarter/ui_start.cpp


