/**
 控制类,控制所有node
*/

#ifndef __SYS_H
#define __SYS_H



#include <string>
#include <map>
#include <vector>

#include "xml.h"
#include "excep.h"
#include "resalloc.h"
#include "ctrlnode.h"
#include "autohd.h"
#include "subsys.h"
#include "modschedul.h"
#include "subdb.h"
//#include "subui.h"


using std::vector;
using std::string;
using std::map;

namespace SCADA
{

class SYS : public CtrlNode
{
public:


    SYS(int argi,char **argb,char **env);
    ~SYS();
    /* 系统启动 */
    int start();
    void stop();
    /* 是否停止标志 */
    int stopFlg() {return mStopFlg;}

    int argc_()const {return mArgc;}
    const char **argv_()  {return mArgv;}
    const char **envp_()  {return mEnvp;}
    /* 站点id 名称 用户名 主机名 */
    const string &id() {return mId;}
    string name() {return mName;}
    void setName(const string &vl) {mName = vl;}
    string user() {return mUser;}
    string host();

    ResRW &UserRes() {return mDataRes;}

    bool cfgFileLoad(); /* 配置文件加载 */
    void cfgFileCheck(bool first = false); /* 定时检查配置修改情况 */

    string icoDir() {return mIconDir;} /** 获取图片路径*/
    string modDir() {return mModDir;} /** 子模块路径 */
    string DBDir() {return mDBDir;} /** 数据库存放目录 */

    /** 配置文件路径 */
    string cfgFile() {return mCfgFile;}
    /** 信号处理函数 */
    static void sighandler(int signal);
    static long long curTime(); /** 当前的时间 */

    AutoHD<SubDB> db() {return at(SUBDB_ID);} /** 数据库*/
    AutoHD<ModSchedul> modSchedul() {return at(MODSCHEDUL_ID);} /** 调度 */
   // AutoHD<SubUI> ui() {return at(SUBUI_ID);} /** 界面操作入口 */
#if 0
    AutoHD<TUIS> ui() {return at("UI");}
    AutoHD<TArchive> archive() {return at("Archive");} /** 存储*/

    AutoHD<TDAQS> daq() {return at("DAQ");} /** 采集 */
    AutoHD<TProtocol> protocol() {return at("Protocol");} /** 协议 */
    AutoHD<TTransport> transport() {return at("Transport");} /** 传输介质 */
    AutoHD<TModSchedul> modSchedul() {return at("ModSched");} /** 调度 */
#endif

    /** 这个节点中存储的是所有的subsys类,目的是可以直接全局访问 */
    void list(vector<string> &list) { chldList(mSubSt,list);}
    bool present(const string &name) {return chldPresent(mSubSt,name);}
    void add(SubSys *sub) {chldAdd(mSubSt,(CtrlNode*)sub);}
    void del(const string &name) {chldDel(mSubSt,name);}
    AutoHD<SubSys> at(const string &name) {return chldAt(mSubSt,name);}

    /** 创建任务 */
    void taskCreate(const string &path,int priority,void *(start_routine)(void *),void *arg);
    void taskDestroy(const string &path,bool *endrunCntr = NULL);
    static void setTaskName(const char *name);
    static void taskSleep(long long per_ms);

protected:
    void load_();
    void save_(); /**/

private:
    /** 任务私有数据 */
    class STask
    {
    public:
                    /*分离*/          /*结束*/
        enum Flgs {Detached = 0x01,FinishTask = 0x02};

        STask():thr(0),policy(0),prior(0),tid(0),flgs(0){}
        STask(pthread_t ithr,char ipolicy,char iprior):thr(ithr),policy(ipolicy),prior(iprior),tid(0),flgs(0){}

        /*属性*/
        string          path;/*线程路径*/
        pthread_t       thr;/*pid*/
        unsigned char   policy,prior;
        pid_t           tid;
        void *(*task)(void *);
        void            *taskArg;
        unsigned int    flgs;

    };
    map<string,STask> mTasks;
    ResRW taskRes;
    static void *taskWrap(void *stas);

    /** sys的私有数据 纯虚函数 必须实现 for nodePath()*/
    const string &nodeName() {return id();}


    /** 子系统id */
    int mSubSt;
    int mStopFlg; /*停止标志 */

    const int mArgc;
    const char **mArgv;
    const char **mEnvp;

    XMLNode rootN;
    time_t cfgTime;

    ResRW mDataRes;
    string mUser; /*用户名 */
    string mCfgFile; /*配置文件名称*/
    string mId; /*站点id */
    string mName; /*站点名称 */
    string mIconDir; /*图片路径*/
    string mModDir; /*子模块库路径*/
    string mDBDir; /* 数据库存放目录 */
};

extern SYS *sys;
}

#endif
