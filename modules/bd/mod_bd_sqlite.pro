TARGET = mod_bd_sqlite
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

HEADERS += \
    $$PWD/sqlite/bd_sqlite.h \
    $$PWD/sqlite/sqlite3.h \
    $$PWD/sqlite/sqlite3ext.h

SOURCES += \
    $$PWD/sqlite/bd_sqlite.cpp \
    $$PWD/sqlite/sqlite3.c
