/**
 ������,��������node
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
    /* ϵͳ���� */
    int start();
    void stop();
    /* �Ƿ�ֹͣ��־ */
    int stopFlg() {return mStopFlg;}

    int argc_()const {return mArgc;}
    const char **argv_()  {return mArgv;}
    const char **envp_()  {return mEnvp;}
    /* վ��id ���� �û��� ������ */
    const string &id() {return mId;}
    string name() {return mName;}
    void setName(const string &vl) {mName = vl;}
    string user() {return mUser;}
    string host();

    ResRW &UserRes() {return mDataRes;}

    bool cfgFileLoad(); /* �����ļ����� */
    void cfgFileCheck(bool first = false); /* ��ʱ��������޸���� */

    string icoDir() {return mIconDir;} /** ��ȡͼƬ·��*/
    string modDir() {return mModDir;} /** ��ģ��·�� */
    string DBDir() {return mDBDir;} /** ���ݿ���Ŀ¼ */

    /** �����ļ�·�� */
    string cfgFile() {return mCfgFile;}
    /** �źŴ����� */
    static void sighandler(int signal);
    static long long curTime(); /** ��ǰ��ʱ�� */

    AutoHD<SubDB> db() {return at(SUBDB_ID);} /** ���ݿ�*/
    AutoHD<ModSchedul> modSchedul() {return at(MODSCHEDUL_ID);} /** ���� */
   // AutoHD<SubUI> ui() {return at(SUBUI_ID);} /** ���������� */
#if 0
    AutoHD<TUIS> ui() {return at("UI");}
    AutoHD<TArchive> archive() {return at("Archive");} /** �洢*/

    AutoHD<TDAQS> daq() {return at("DAQ");} /** �ɼ� */
    AutoHD<TProtocol> protocol() {return at("Protocol");} /** Э�� */
    AutoHD<TTransport> transport() {return at("Transport");} /** ������� */
    AutoHD<TModSchedul> modSchedul() {return at("ModSched");} /** ���� */
#endif

    /** ����ڵ��д洢�������е�subsys��,Ŀ���ǿ���ֱ��ȫ�ַ��� */
    void list(vector<string> &list) { chldList(mSubSt,list);}
    bool present(const string &name) {return chldPresent(mSubSt,name);}
    void add(SubSys *sub) {chldAdd(mSubSt,(CtrlNode*)sub);}
    void del(const string &name) {chldDel(mSubSt,name);}
    AutoHD<SubSys> at(const string &name) {return chldAt(mSubSt,name);}

    /** �������� */
    void taskCreate(const string &path,int priority,void *(start_routine)(void *),void *arg);
    void taskDestroy(const string &path,bool *endrunCntr = NULL);
    static void setTaskName(const char *name);
    static void taskSleep(long long per_ms);

protected:
    void load_();
    void save_(); /**/

private:
    /** ����˽������ */
    class STask
    {
    public:
                    /*����*/          /*����*/
        enum Flgs {Detached = 0x01,FinishTask = 0x02};

        STask():thr(0),policy(0),prior(0),tid(0),flgs(0){}
        STask(pthread_t ithr,char ipolicy,char iprior):thr(ithr),policy(ipolicy),prior(iprior),tid(0),flgs(0){}

        /*����*/
        string          path;/*�߳�·��*/
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

    /** sys��˽������ ���麯�� ����ʵ�� for nodePath()*/
    const string &nodeName() {return id();}


    /** ��ϵͳid */
    int mSubSt;
    int mStopFlg; /*ֹͣ��־ */

    const int mArgc;
    const char **mArgv;
    const char **mEnvp;

    XMLNode rootN;
    time_t cfgTime;

    ResRW mDataRes;
    string mUser; /*�û��� */
    string mCfgFile; /*�����ļ�����*/
    string mId; /*վ��id */
    string mName; /*վ������ */
    string mIconDir; /*ͼƬ·��*/
    string mModDir; /*��ģ���·��*/
    string mDBDir; /* ���ݿ���Ŀ¼ */
};

extern SYS *sys;
}

#endif
