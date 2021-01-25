#include "sys.h"
#include "ui_main.h"


#define QTMAIN_MOD_ID       "QTMain"
#define QTMAIN_MOD_NAME     "qt main"
#define QTMAIN_MOD_TYPE     SUBUI_ID
#define QTMAIN_VER_TYPE     SUBUI_VER
#define QTMAIN_SUB_TYPE     "QT"
#define QTMAIN_VERSION      "0.0.1"
#define QTMAIN_AUTORS       "BigTao"
#define QTMAIN_DESCRIPTION  "qt main"
#define QTMAIN_LICENSE      "GPL2"


QTMain::StartMain *QTMain::mod;

extern "C"
{
    Module::SAt module( int n_mod )
    {
        if( n_mod==0 )	return Module::SAt(QTMAIN_MOD_ID,QTMAIN_MOD_TYPE,QTMAIN_VER_TYPE);
        return Module::SAt("");
    }

    Module *attach( const Module::SAt &AtMod, const string &source )
    {
        if( AtMod == Module::SAt(QTMAIN_MOD_ID,QTMAIN_MOD_TYPE,QTMAIN_VER_TYPE) )
            return new QTMain::StartMain( source );
        return NULL;
    }
}

using namespace QTMain;

StartMain::StartMain(string name):ModUI(QTMAIN_MOD_ID)
{
    mod		= this;

    mName	= QTMAIN_MOD_NAME;
    mType	= QTMAIN_MOD_TYPE;
    mVers	= QTMAIN_VERSION;
    mAutor	= QTMAIN_AUTORS;
    mDescr	= QTMAIN_DESCRIPTION;
    mLicense	= QTMAIN_LICENSE;
    mSource	= name;

    //Public export functions
    modFuncReg( new ExpFunc("QMainWindow *openWindow();","Start QT GUI.",(void(Module::*)( )) &StartMain::openWindow) );
}

StartMain::~StartMain()
{
    if(run_st)
        modStop();
}

string StartMain::modInfo(const std::string &name)
{
    if(name == "SubType")
        return QTMAIN_SUB_TYPE;
    else
        return Module::modInfo(name);
}

void StartMain::modInfo(vector<std::string> &list)
{
    Module::modInfo(list);
    list.push_back("SubType");
}

QMainWindow *StartMain::openWindow()
{
    return new QMainWindow();
}
