TEMPLATE = subdirs


SUBDIRS += $$PWD/src/scada.pro \
           $$PWD/called/main.pro \
           $$PWD/modules/modules.pro \

DISTFILES += \
    cfg/config.xml

