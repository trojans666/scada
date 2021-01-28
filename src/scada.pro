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



OBJECTS_DIR = $$PWD/../pro/scada

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
    $$PWD/mess/priv/appender.h \
    $$PWD/mess/priv/appender_type_mmap.h \
    $$PWD/mess/priv/appender_type_rollingfile.h \
    $$PWD/mess/priv/appender_type_stream.h \
    $$PWD/mess/priv/appender_type_stream2.h \
    $$PWD/mess/priv/appender_type_syslog.h \
    $$PWD/mess/priv/buffer.h \
    $$PWD/mess/priv/category.h \
    $$PWD/mess/priv/error.h \
    $$PWD/mess/priv/factory.h \
    $$PWD/mess/priv/hash.h \
    $$PWD/mess/priv/init.h \
    $$PWD/mess/priv/layout.h \
    $$PWD/mess/priv/layout_type_basic.h \
    $$PWD/mess/priv/layout_type_basic_r.h \
    $$PWD/mess/priv/layout_type_dated.h \
    $$PWD/mess/priv/layout_type_dated_r.h \
    $$PWD/mess/priv/list.h \
    $$PWD/mess/priv/location_info.h \
    $$PWD/mess/priv/logging_event.h \
    $$PWD/mess/priv/priority.h \
    $$PWD/mess/priv/rc.h \
    $$PWD/mess/priv/rollingpolicy.h \
    $$PWD/mess/priv/rollingpolicy_type_sizewin.h \
    $$PWD/mess/priv/sprintf.h \
    $$PWD/mess/priv/stack.h \
    $$PWD/mess/mess.h \
    $$PWD/subsys/subtransport.h \
    $$PWD/subsys/subprotocol.h



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
    $$PWD/mess/priv/appender.c \
    $$PWD/mess/priv/appender_type_mmap.c \
    $$PWD/mess/priv/appender_type_rollingfile.c \
    $$PWD/mess/priv/appender_type_stream.c \
    $$PWD/mess/priv/appender_type_stream2.c \
    $$PWD/mess/priv/appender_type_syslog.c \
    $$PWD/mess/priv/category.c \
    $$PWD/mess/priv/error.c \
    $$PWD/mess/priv/factory.c \
    $$PWD/mess/priv/hash.c \
    $$PWD/mess/priv/init.c \
    $$PWD/mess/priv/layout.c \
    $$PWD/mess/priv/layout_type_basic.c \
    $$PWD/mess/priv/layout_type_basic_r.c \
    $$PWD/mess/priv/layout_type_dated.c \
    $$PWD/mess/priv/layout_type_dated_r.c \
    $$PWD/mess/priv/list.c \
    $$PWD/mess/priv/logging_event.c \
    $$PWD/mess/priv/priority.c \
    $$PWD/mess/priv/rollingpolicy.c \
    $$PWD/mess/priv/rollingpolicy_type_sizewin.c \
    $$PWD/mess/priv/sprintf.c \
    $$PWD/mess/priv/stack.c \
    $$PWD/mess/mess.cpp \
    $$PWD/subsys/subtransport.cpp




