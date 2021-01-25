#ifndef __UI_START_H
#define __UI_START_H

#include <QObject>
#include <QTranslator>
#include <QTimer>

#include "subui.h"

using namespace SCADA;

namespace QTStarter
{

class WinControl : public QObject
{
    Q_OBJECT
public:
    WinControl();

    bool callQTModule(const string &nm);
    void startDialog();

private slots:
    void lastWinClose();

protected:

private:

};

/* mod ui */
class ModQTStart : public ModUI
{
public:
    ModQTStart(string name);
    ~ModQTStart();

    bool startCom() {return start_com;}
    string startMod()   {return start_mod;}

    void setStartMod(const string &vl)  {start_mod = vl;}

    void modStart();
    void modStop();

protected:
    void load_();
    void save_() {}

private:
    static void *Task(void *);

    bool start_com;
    string start_mod;
};

extern ModQTStart *mod;
}

#endif
