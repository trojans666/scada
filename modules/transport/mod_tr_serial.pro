TARGET = mod_tr_serial
TEMPLATE = lib

CONFIG += plugin

QT -= gui core

# CONFIG += release #debug_and_release debug

DESTDIR = $$PWD/../../output/modules


unix {
    target.path = $$DESTDIR
    INSTALLS += target
}

LIBS += -L$$PWD/../../output -lscada

OBJECTS_DIR = $$PWD/../../pro/obj

INCLUDEPATH += $$PWD/../../src/core \
            $$PWD/../../src/event \
            $$PWD/../../src/excep \
            $$PWD/../../src/subsys \
            $$PWD/../../src/xml

SOURCES += \
    serial/tr_serial.cpp

HEADERS += \
    serial/tr_serial.h


