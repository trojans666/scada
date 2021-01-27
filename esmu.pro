TEMPLATE = subdirs


SUBDIRS += $$PWD/src/scada.pro \
           $$PWD/called/main.pro \
           $$PWD/modules/bd/mod_bd_sqlite.pro \
           $$PWD/modules/ui/mod_ui_qtstart.pro \
           #$$PWD/modules/ui/mod_ui_qtmain.pro \

