
#include "subui.h"

using namespace SCADA;

SubUI::SubUI() : SubSys(SUBUI_ID,SUBUI_NAME,true)
{

}

void SubUI::subStart()
{
    SubSys::subStart();
}

void SubUI::subStop()
{
    SubSys::subStop();
}


ModUI::ModUI(const std::string &id) : Module(id),run_st(false)
{

}

