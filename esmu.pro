TEMPLATE = subdirs


SUBDIRS += $$PWD/src/scada.pro \
           $$PWD/called/main.pro \
           $$PWD/modules/bd/mod_bd_sqlite.pro \
           $$PWD/modules/ui/mod_ui_qtstart.pro \
           $$PWD/modules/transport/mod_tr_serial.pro \
           $$PWD/modules/transport/mod_tr_socket.pro \
           $$PWD/modules/transport/mod_tr_can.pro \
           $$PWD/modules/ui/mod_ui_qtmain.pro \

