/**
界面操作类
*/

#ifndef __SUBUI_H
#define __SUBUI_H

#include "subsys.h"
#include "module.h"

#define SUBUI_VER       1

#define SUBUI_ID        "UI"
#define SUBUI_NAME      "User interfaces"

namespace SCADA
{


class ModUI : public Module
{
public:
    ModUI(const string &id);
    bool startStat() {return run_st;}

protected:
    bool run_st;
};

class SubUI : public SubSys
{
public:
    SubUI();
    int subVer()    {return SUBUI_VER;}
    /** 主要是这两个 函数 */
    void subStart();
    void subStop();

    AutoHD<ModUI> at(const string &name) {return modAt(name);}


protected:
    void load_() {}
private:

};

}

#endif

