TARGET = scada
TEMPLATE = lib

CONFIG += plugin

QT -= gui core

# CONFIG += release #debug_and_release debug

DESTDIR = $$PWD/../output

LIBS += -lpthread -ldl

unix {
    target.path = $$DESTDIR
    INSTALLS += target
}



OBJECTS_DIR = $$PWD/../pro/obj

INCLUDEPATH += $$PWD/core \
            $$PWD/event \
            $$PWD/excep \
            $$PWD/subsys \
            $$PWD/xml

HEADERS += \
    $$PWD/core/autohd.h \
    $$PWD/core/ctrlnode.h \
    $$PWD/core/log.h \
    $$PWD/core/modschedul.h \
    $$PWD/core/module.h \
    $$PWD/core/resalloc.h \
    $$PWD/core/stropt.h \
    $$PWD/core/subsys.h \
    $$PWD/core/sys.h \
    $$PWD/event/event.h \
    $$PWD/excep/excep.h \
    $$PWD/subsys/subdb.h \
    $$PWD/subsys/subui.h \
    $$PWD/xml/priv/ascii.h \
    $$PWD/xml/priv/asciitab.h \
    $$PWD/xml/priv/expat.h \
    $$PWD/xml/priv/iasciitab.h \
    $$PWD/xml/priv/latin1tab.h \
    $$PWD/xml/priv/nametab.h \
    $$PWD/xml/priv/utf8tab.h \
    $$PWD/xml/priv/xmlrole.h \
    $$PWD/xml/priv/xmltok.h \
    $$PWD/xml/priv/xmltok_impl.h \
    $$PWD/xml/xml.h \
    subsys/subarchive.h \
    mess/priv/logobject.h \
    mess/priv/logobjectptr.h \
    mess/priv/filter.h \
    mess/priv/level.h \
    mess/priv/loggingevent.h \
    mess/priv/appender.h \
    mess/priv/layout.h


SOURCES += \
    $$PWD/core/ctrlnode.cpp \
    $$PWD/core/log.cpp \
    $$PWD/core/modschedul.cpp \
    $$PWD/core/module.cpp \
    $$PWD/core/resalloc.cpp \
    $$PWD/core/stropt.cpp \
    $$PWD/core/subsys.cpp \
    $$PWD/core/sys.cpp \
    $$PWD/excep/excep.cpp \
    $$PWD/subsys/subdb.cpp \
    $$PWD/subsys/subui.cpp \
    $$PWD/xml/xml.cpp \
    $$PWD/event/event.c \
    $$PWD/xml/priv/xmlparse.c \
    $$PWD/xml/priv/xmlrole.c \
    $$PWD/xml/priv/xmltok.c \
    subsys/subarchive.cpp \
    mess/priv/filter.cpp \
    mess/priv/layout.cpp

