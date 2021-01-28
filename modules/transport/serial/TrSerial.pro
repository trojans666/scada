TARGET = TrSerial
TEMPLATE = lib

CONFIG += plugin

QT -= gui core

# CONFIG += release #debug_and_release debug

DESTDIR = $$PWD/../../../output/modules


unix {
    target.path = $$DESTDIR
    INSTALLS += target
}

LIBS += -L$$PWD/../../../output -lscada

OBJECTS_DIR = $$PWD/../../../pro/trserial

INCLUDEPATH += $$PWD/../../../src/core \
            $$PWD/../../../src/event \
            $$PWD/../../../src/excep \
            $$PWD/../../../src/subsys \
            $$PWD/../../../src/xml

SOURCES += \
    tr_serial.cpp

HEADERS += \
    tr_serial.h


