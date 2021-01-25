#include <unistd.h>


#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QTextCodec>
#include <QSplashScreen>
#include <QFrame>
#include <QVBoxLayout>
#include <QMenu>
#include <QToolBar>
#include <QMenuBar>

#include "sys.h"
#include "stropt.h"
#include "log.h"
#include "ui_start.h"

/********* attribute *******/
#define QT_MOD_ID       "QTStarter"
#define QT_MOD_NAME     "QT GUI starter"
#define QT_MOD_TYPE     SUBUI_ID
#define QT_VER_TYPE     SUBUI_VER
#define QT_VERSION      "0.0.1"
#define QT_AUTORS       "BigTao"
#define QT_DESCRIPTION  "single for all QT GUI modules"
#define QT_LICENSE      "GPL2"


QTStarter::ModQTStart *QTStarter::mod;

extern "C"
{
    Module::SAt module(int n_mod)
    {
        if(n_mod == 0)
            return Module::SAt(QT_MOD_ID,QT_MOD_TYPE,QT_VER_TYPE);
        return Module::SAt("");
    }

    Module *attach(const Module::SAt &AtMod,const string &source)
    {
        if(AtMod == Module::SAt(QT_MOD_ID,QT_MOD_TYPE,QT_VER_TYPE))
            return new QTStarter::ModQTStart(source);
        return NULL;
    }
}

using namespace QTStarter;

ModQTStart::ModQTStart(std::string name):ModUI(QT_MOD_ID),start_com(false)
{
    mod = this;

    mName = QT_MOD_NAME;
    mType = QT_MOD_TYPE;
    mVers = QT_VERSION;
    mAutor = QT_AUTORS;
    mDescr = QT_DESCRIPTION;
    mLicense = QT_LICENSE;
    mSource = name;
}

ModQTStart::~ModQTStart()
{
    if(run_st)
        modStop();
}

void ModQTStart::load_()
{

}

void ModQTStart::modStart()
{
   if(!run_st)
   {
        mess_info("modStart","qtstart modstart");
       sys->taskCreate(nodePath('.',true) + "qtstart",0,Task,this);
   }
}

void ModQTStart::modStop()
{
    start_com = false;
    if(run_st)
    {
        mess_info("modstop","qtstart stop");
       sys->taskDestroy(nodePath('.',true) + "qtstart",&run_st);
    }

}

void *ModQTStart::Task(void *)
{
    mess_info("Task","task start....");

    bool first_ent = true;
    QImage ico_t;
    time_t st_time = time(NULL);

    /* en for string */
    QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale () );

    QApplication *QtApp = new QApplication(sys->argc_(),sys->argv_());
    /* The program exits when the last window exits */

    mod->run_st = true;

    QTranslator translator;
    QtApp->installTranslator(&translator);

    /* start splash */
    ico_t.load("icon/QTStarter_splash.png");
    QSplashScreen *splash = new QSplashScreen(QPixmap::fromImage(ico_t));
    splash->show();
    QFont wFnt = splash->font();
    wFnt.setPixelSize(10);
    splash->setFont(wFnt);

    /* 启动中 加载中  */
    QString mess = "Starting... Please Wait!!!";
    int i = 3;
    do
    {
       mess_info("splash","!!!!!!!!!!!");
       splash->showMessage(mess,Qt::AlignBottom | Qt::AlignLeft);
       QtApp->processEvents();
       usleep(1000 * 1000); /* 500 ms */
    }while(i--);

    delete splash;

    /* start external modules */
    WinControl *winCntr = new WinControl();

    mess_info("wincntr","$$$$$$$");
    /* start call dialog */
    if(QApplication::topLevelWidgets().isEmpty())
        winCntr->startDialog();
    mess_info("wincntr","22222");
    QObject::connect(QtApp,SIGNAL(lastWindowClosed()),winCntr,SLOT(lastWinClose()));

    mess_info("exec","####");
    QtApp->exec();

    /* stop over ***/
    delete winCntr;

    /* stop splash cretae */
    ico_t.load("icon/QTStarter_splash1.png");
    splash = new QSplashScreen(QPixmap::fromImage(ico_t));
    splash->show();
    splash->setFont(wFnt);

    st_time = time(NULL);

    /* show message */


    delete splash;

    /* free application */
    delete QtApp;
    first_ent = false;

    mod->run_st = false;

    return NULL;
}

/* priv slots */
void WinControl::lastWinClose()
{
    if(sys->stopFlg())
    {
        mess_info("lastwinclose","app quit...");
        qApp->quit(); /* stop */
    }
    else
        startDialog(); /* restart */
}

/*** wincontrol */
WinControl::WinControl()
{

}

void WinControl::startDialog()
{
    vector<string> list;
    mod->owner().modList(list);
    unsigned int i_l;
    mess_info("startdialog","##%d",list.size());
    for( i_l = 0;i_l < list.size();i_l++)
    {
        mess_info("modat","@@%s",mod->owner().modAt(list[i_l]).at().modInfo("SubType").c_str());
        if(mod->owner().modAt(list[i_l]).at().modInfo("SubType") == "QT" &&
                mod->owner().modAt(list[i_l]).at().modFuncPresent("QMainWindow *openWindow();"))
        {
            mess_info("for","$$%d",i_l);
                break; /* first match*/
        }
        mess_info("for","fjslksfj");
    }

    if(i_l >= list.size())
        return ;

    AutoHD<Module> qt_mod = mod->owner().modAt(list[i_l]);
    QMainWindow *(Module::*openWindow)();
    mess_info("for","fjslksfj11");
    qt_mod.at().modFunc("QMainWindow *openWindow();",(void (Module::**)()) &openWindow);
    QMainWindow *new_wnd = ((&qt_mod.at())->*openWindow)( );
    mess_info("for","fjslksfj22");
    if(!new_wnd)
        return ;

    /* start */
     mess_info("show","show over");
    new_wnd->show();
    mess_info("show","show over");
}
