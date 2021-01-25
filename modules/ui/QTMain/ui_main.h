#ifndef __UI_MAIN_H
#define __UI_MAIN_H

#include <QMainWindow>

#include "subui.h"

using namespace SCADA;

namespace QTMain
{

class StartMain : public ModUI
{
public:
    StartMain(string name);
    ~StartMain();
    string modInfo(const string &name);
    void modInfo(vector<string> &list);

protected:

private:
    QMainWindow *openWindow();
};

extern StartMain *mod;
}

#endif
