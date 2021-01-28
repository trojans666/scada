TARGET = mod_bd_sqlite
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

OBJECTS_DIR = $$PWD/../../../pro/bdsqlite

INCLUDEPATH += $$PWD/../../../src/core \
            $$PWD/../../../src/event \
            $$PWD/../../../src/excep \
            $$PWD/../../../src/subsys \
            $$PWD/../../../src/xml

HEADERS += \
    bd_sqlite.h \
    sqlite3.h \
    sqlite3ext.h

SOURCES += \
    bd_sqlite.cpp \
    sqlite3.c
