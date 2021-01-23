#include <QApplication>

#include "sys.h"
#include "stropt.h"
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

ModQTStart::ModQTStart(std::string name):ModUI(QT_MOD_ID),end_run(false),demon_mode(false), start_com(false)
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

void ModQTStart::modStart()
{
    if(!run_st)
    {
        sys->taskCreate(nodePath('.',true),0,Task,this);
    }
}

void ModQTStart::modStop()
{
    if(run_st)
    {
       sys->taskDestroy(nodePath('.',true),&run_st);
    }
}

void *ModQTStart::Task(void *)
{
    vector<string> list;
    bool first_ent = true;
    QImage ico_t;
    time_t st_time = time(NULL);

    QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale () );

    QApplication *QtApp = new QApplication(sys->argc_(),sys->argv_());
    QtApp->setQuitOnLastWindowClosed(false);
    mod->run_st = true;

    QTranslator translator;
    QtApp->installTranslator(&translator);

    /* start splash */
    ico_t.load();
    QSplashScreen *splash = new QSplashScreen(QPixmap::fromImage(ioc_t));
    splash->show();
    QFont wFnt = splash->font();
    wFnt.setPixelSize(10);
    splash->setFont(wFnt);

    /* 启动中 加载中  */

    delete splash;

    /* start external modules */
    WinControl *winCntr = new WinControl();

    int op_wnd = 0;
    mod->owner().modList(list);
    for(unsigned int i_l = 0;i_l < list.size();i_l++)
    {
        if(mod->owner().modAt(list[i_l]).at().modInfo("SubType") == "QT" &&
                mod->owner().modAt(list[i_l]).at().modFuncPresent("QMainWindow *openWinwod();"))
        {
            int i_off = 0;
            string s_el;
            while((s_el = StrOpt::strSepParse(mod->start_mod,0,';',&i_off)).size())
            {
                if(s_el == list[i_l])
                    break;
                if(!s_el.empty() || !i_off)
                {
                    if(winCntr->callQTModule(list[i_l]))
                        op_wnd++;
                }
            }
        }
    }

    /* start call dialog */
    if(QApplication::topLevelWidgets().isEmpty())
        winCntr->startDialog();

    QtApp->exec();

    /* stop over ***/
    delete winCntr;

    /* stop splash cretae */
    ico_t.load();
    splash = new QSplashScreen(QPixmap::fromImage(ico_t));
    splash->show();
    splash->setFont(wFnt);

    st_time = time(NULL);

    /* 打印信息  */
    delete splash;

    /* free application */
    delete QtApp;
    first_ent = false;

    mod->run_st = false;

    return NULL;
}

/*** wincontrol */
WinControl::WinControl()
{

}

bool WinControl::callQTModule(const std::string &nm)
{
    vector<string> list;

    AutoHD<TModule> qt_mod = mod->owner().modAt(nm);
    QMainWindow *(TModule::*openWindow)( );
    qt_mod.at().modFunc("QMainWindow *openWindow();",(void (Module::**)()) &openWindow);
    QMainWindow *new_wnd = ((&qt_mod.at())->*openWindow)( );
    if( !new_wnd ) return false;

    //> Make QT starter toolbar
    QToolBar *toolBar = NULL;
    QMenu *menu = NULL;
    if( !new_wnd->property("QTStarterToolDis").toBool() )
    {
    toolBar = new QToolBar("QTStarter",new_wnd);
    toolBar->setObjectName("QTStarterTool");
    new_wnd->addToolBar(toolBar);
    }
    if( !new_wnd->property("QTStarterMenuDis").toBool() && !new_wnd->menuBar()->actions().empty() )
    menu = new_wnd->menuBar()->addMenu("QTStarter");

    mod->owner().modList(list);
    for( unsigned i_l = 0; i_l < list.size(); i_l++ )
    if( mod->owner().modAt(list[i_l]).at().modInfo("SubType") == "QT" &&
        mod->owner().modAt(list[i_l]).at().modFuncPresent("QMainWindow *openWindow();") )
    {
    AutoHD<TModule> qt_mod = mod->owner().modAt(list[i_l]);

    QIcon icon;
    if( mod->owner().modAt(list[i_l]).at().modFuncPresent("QIcon icon();") )
    {
        QIcon(TModule::*iconGet)();
        mod->owner().modAt(list[i_l]).at().modFunc("QIcon icon();",(void (TModule::**)()) &iconGet);
        icon = ((&mod->owner().modAt(list[i_l]).at())->*iconGet)( );
    }
    else icon = QIcon(":/images/oscada_qt.png");
    QAction *act_1 = new QAction(icon,qt_mod.at().modName().c_str(),new_wnd);
    act_1->setObjectName(list[i_l].c_str());

    act_1->setToolTip(qt_mod.at().modName().c_str());
    act_1->setWhatsThis(qt_mod.at().modInfo("Description").c_str());
    QObject::connect(act_1, SIGNAL(activated()), this, SLOT(callQTModule()));

    if( toolBar ) toolBar->addAction(act_1);
    if( menu ) menu->addAction(act_1);
    }

    new_wnd->show();

    return true;
}

void WinControl::startDialog()
{
    vector<string> list;

    QMainWindow *new_wnd = new QMainWindow( );
    new_wnd->setWindowTitle(_("OpenSCADA system QT-starter"));
    new_wnd->setWindowIcon(QIcon(":/images/oscada_qt.png"));

    new_wnd->setCentralWidget( new QWidget(new_wnd) );
    QVBoxLayout *new_wnd_lay = new QVBoxLayout(new_wnd->centralWidget());
    new_wnd_lay->setMargin(6);
    new_wnd_lay->setSpacing(4);

    mod->owner().modList(list);
    for( unsigned i_l = 0; i_l < list.size(); i_l++ )
        if( mod->owner().modAt(list[i_l]).at().modInfo("SubType") == "QT" &&
                mod->owner().modAt(list[i_l]).at().modFuncPresent("QMainWindow *openWindow();") )
        {
            QIcon icon;
            if( mod->owner().modAt(list[i_l]).at().modFuncPresent("QIcon icon();") )
            {
                QIcon (Module::*iconGet)();
                mod->owner().modAt(list[i_l]).at().modFunc("QIcon icon();",(void (Module::**)()) &iconGet);
                icon = ((&mod->owner().modAt(list[i_l]).at())->*iconGet)( );
            }
            else icon = QIcon(":/images/oscada_qt.png");

            AutoHD<Module> qt_mod = mod->owner().modAt(list[i_l]);
            QPushButton *butt = new QPushButton(icon,qt_mod.at().modName().c_str(),new_wnd->centralWidget());
            butt->setObjectName(list[i_l].c_str());
            //QObject::connect(butt, SIGNAL(clicked(bool)), this, SLOT(callQTModule()));
            new_wnd_lay->addWidget( butt, 0, 0 );
        }

    new_wnd_lay->addItem( new QSpacerItem( 20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

    QFrame *gFrame = new QFrame( new_wnd->centralWidget() );
    gFrame->setFrameShape(QFrame::HLine);
    gFrame->setFrameShadow(QFrame::Raised);
    new_wnd_lay->addWidget(gFrame,0,0);

    QPushButton *butt = new QPushButton(QIcon(":/images/exit.png"),_("Exit from system"), new_wnd->centralWidget());
    butt->setObjectName("*exit*");
    //QObject::connect(butt, SIGNAL(clicked(bool)), this, SLOT(callQTModule()));
    new_wnd_lay->addWidget( butt, 0, 0 );

    new_wnd->show();
}
