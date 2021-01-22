#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdarg.h>

#ifdef __linux__
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif // __linux__

#include "log.h"
#include "stropt.h"

#include "sys.h"
#include <iostream>

using namespace SCADA;

SYS *SCADA::sys;
Log *SCADA::mLog;

SYS::SYS(int argi, char **argb, char **env)
    :mStopFlg(-1)
    ,mArgc(argi)
    ,mArgv((const char **)argb)
    ,mEnvp((const char **)env)
    ,mUser("root")
    ,mCfgFile("cfg/config.xml")
    ,mId("Scada_ID")
    ,mName("Scada_Name")
    ,mIconDir("pic/")
    ,mModDir("modules")
{
    sys = this;
    mSubSt = grpAdd("sub_",true); /*true ʱ������ */
    mLog = new Log();

    /** set signal */
    signal(SIGINT,sighandler);
    signal(SIGTERM,sighandler);
    signal(SIGALRM,sighandler);
    signal(SIGPIPE,sighandler);
    signal(SIGABRT,sighandler);
    signal(SIGCHLD,sighandler);
    signal(SIGFPE,sighandler);
    signal(SIGSEGV,sighandler);


}

SYS::~SYS()
{
    del(SUBDB_ID);
    if(mLog)
        delete mLog;
}

string SYS::host()
{
    utsname ubuf;
    uname(&ubuf);
    return ubuf.nodename;
}

bool SYS::cfgFileLoad()
{
    int hd = open(mCfgFile.c_str(),O_RDONLY);
    if(hd < 0)
    {
        return false;
    }
    string s_buf;
    int cf_sz = lseek(hd,0,SEEK_END);
    if(cf_sz > 0)
    {
        lseek(hd,0,SEEK_SET);
        char *buf = (char *)malloc(cf_sz + 1);
        if(!buf)
        {
            mess_debug("cfgFileLoad","malloc failed");
            return false;
        }
        (void)read(hd,buf,cf_sz);
        buf[cf_sz] = 0;
        s_buf = buf;
        free(buf);
    }
    close(hd);

    try
    {
        ResAlloc res(UserRes(),true);
        rootN.load(s_buf);
        if(rootN.name() == "root")
        {
            XMLNode *stat_n = NULL;
            for(int i_st = rootN.childSize()-1; i_st >= 0; i_st--)
            {
                if(rootN.childGet(i_st)->name() == "station")
                {
                    stat_n = rootN.childGet(i_st);
                    if(stat_n->attr("id") == mId)
                        break;
                }
            }
            if(stat_n && stat_n->attr("id") != mId)
            {
                mId = stat_n->attr("id");
            }
            if(!stat_n)
                rootN.clear();
        }
        else
        {
            rootN.clear();
        }
    }
    catch(TError err)
    {

    }

    return true;
}

/** �������� */
void SYS::taskCreate(const string &path,int priority,void *(start_routine)(void *),void *arg)
{
    int detachStat = 0;
    pthread_t procPthr;
    pthread_attr_t locPAttr, *pthr_attr;

    map<string,STask>::iterator ti;

    ResAlloc res(taskRes, true); /*���� ִ�����Զ�����*/

    for(time_t c_tm = time(NULL); (ti=mTasks.find(path)) != mTasks.end(); )
    {
        //ɾ���Ѵ���������ɵ�δ���ٵ�����
        if(ti->second.flgs&STask::FinishTask && !(ti->second.flgs&STask::Detached))
        {
            pthread_join(ti->second.thr, NULL);
            mTasks.erase(ti);
            continue;
        }

        res.release(); /*�ͷ���*/

        //��ǰ��������
        if(time(NULL) >= (c_tm+5))
            return ;

        usleep(10 * 1000); /*10ms*/

        res.request(true);/*�ټ�����������һ����ѯ*/
    }

    STask &htsk = mTasks[path];
    htsk.path = path;
    htsk.task = start_routine;
    htsk.taskArg = arg;
    htsk.flgs = 0;
    htsk.thr = 0;
    htsk.prior = priority%100; /*0~99*/

    res.release();

//    if(pAttr)
//    {
//        pthr_attr = pAttr;
//    }
//    else
    {
        pthr_attr = &locPAttr;
        pthread_attr_init(pthr_attr); /*Ĭ������*/
    }

    /*�µ��̼̳߳в��ԺͲ���������schedpolicy��schedparam��������ʽ���õĵ�����Ϣ*/
    pthread_attr_setinheritsched(pthr_attr, PTHREAD_EXPLICIT_SCHED);

    struct sched_param prior;
    prior.sched_priority = 0;

    int policy = SCHED_OTHER; /*Ĭ�Ϸ�ʱ����*/

    if(priority > 0)
        policy = SCHED_RR; /*ʱ��Ƭ*/
    if(priority >= 100)
        policy = SCHED_FIFO; /*ʵʱ*/
    pthread_attr_setschedpolicy(pthr_attr, policy);

    prior.sched_priority = vmax(sched_get_priority_min(policy), vmin(sched_get_priority_max(policy),priority%100));

    pthread_attr_setschedparam(pthr_attr, &prior);//���ȼ��趨

    try
    {
        pthread_attr_getdetachstate(pthr_attr,&detachStat);

        if(detachStat == PTHREAD_CREATE_DETACHED)
            htsk.flgs |= STask::Detached; /*�ж��Ƿ��̷߳���*/

        int rez = pthread_create(&procPthr, pthr_attr, taskWrap, &htsk);

        if(rez == EPERM) /*����������*/
        {
            policy = SCHED_OTHER;
            pthread_attr_setschedpolicy(pthr_attr, policy);
            prior.sched_priority = 0;
            pthread_attr_setschedparam(pthr_attr,&prior);

            rez = pthread_create(&procPthr, pthr_attr, taskWrap, &htsk);
        }

        if(pthr_attr)
            pthread_attr_destroy(pthr_attr);

        if(rez)
            throw "Task creation error";

        //�ȴ��߳̽ṹ��ʼ����ɣ�����ɲ��ɷ��������
        while(!(htsk.flgs&STask::Detached) && !htsk.thr)
            usleep(30 * 1000);

//        //�ȴ�����״̬ startSt = NULL
//        for(time_t c_tm = time(NULL); !(htsk.flgs&STask::Detached) && startSt && !(*startSt); )
//        {
//            if(time(NULL) >= (c_tm+wtm))
//                throw "Task start timeouted!";

//            usleep(100 * 1000);
//        }
    }
    catch(const char *msg)
    {
        if(1)  		//ɾ��pthread_create()����Ϣ�������Ժ���ܿ�ʼʱ����������������
        {
            res.request(true);
            mTasks.erase(path);
            res.release();
        }
        throw msg;
    }
}

void *SYS::taskWrap(void *stas)
{
    STask *tsk = (STask *)stas;


    void *(*wTask) (void *) = tsk->task;
    void *wTaskArg = tsk->taskArg;


    int policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    tsk->policy = policy;
    //tsk->prior = param.sched_priority;


    if(tsk->policy != SCHED_RR && tsk->prior > 0 && setpriority(PRIO_PROCESS,tsk->tid,-tsk->prior/5) != 0)
        tsk->prior = 0;
    tsk->thr = pthread_self();


    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);


    void *rez = NULL;
    rez = wTask(wTaskArg);

    tsk->flgs |= STask::FinishTask;

    //> Remove task object for detached
    if(tsk->flgs & STask::Detached)
        sys->taskDestroy(tsk->path);

    return rez;
}

void SYS::taskDestroy(const string &path,bool *endrunCntr)
{
    map<string,STask>::iterator it;

    ResAlloc res(taskRes, false); /*����ʱ�Զ��ͷ�������*/

    if(mTasks.find(path) == mTasks.end())
        return;

    res.release();/*�ͷ���*/

    if(endrunCntr)
        *endrunCntr = true; /*�����Ѿ����Խ����߳���*/

    //�ȴ��������
    time_t t_tm, s_tm;
    t_tm = s_tm = time(NULL);


    res.request(true); /*д��*/

    while((it=mTasks.find(path)) != mTasks.end() && !(it->second.flgs&STask::FinishTask))
    {
        res.release();

        time_t c_tm = time(NULL);

        //�鿴�Ƿ�ʱ
        if(c_tm > (s_tm+5))
        {
            return ;
        }

        //
        if(c_tm > t_tm+1)
        {
            //1sec
            t_tm = c_tm;
        }
        usleep(100 * 1000);/*100ms*/

        res.request(true);
    }
    if(it != mTasks.end())
    {
        if(!(it->second.flgs&STask::Detached))
            pthread_join(it->second.thr, NULL);
        mTasks.erase(it);
    }
}

void SYS::setTaskName(const char *name)
{
    if(!name)
        return ;

    prctl(PR_SET_NAME,name);
}

void SYS::taskSleep(long long per_ms)
{
    usleep(per_ms * 1000);
}



void SYS::load_()
{
    static bool first_load = true;

    bool rez = cfgFileLoad(); /* �����ļ����� */

    if(first_load)
    {
        add(new SubDB());
        add(new ModSchedul());

        /* load modules */
        modSchedul().at().load(); /* ÿ���඼�̳��Ը��� ctrlnode �������load�����load_ (�麯��) */
        if(! modSchedul().at().loadLibS())
        {
            stop();
        }

        /* load dbs */
        // db().at().load();
        mess_info("ss","1111");
        /* ֱ�Ӽ������Ե���ϵͳ��module */
        vector<string> lst;
        list(lst);

        for(unsigned i_a = 0; i_a < lst.size(); i_a++)
        {
            try
            {
                at(lst[i_a]).at().load();
            }
            catch(TError err)
            {

            }
        }
    }
    if(!rez)
        stop();
    first_load = false;
}
void SYS::save_() /**/
{

}

long long SYS::curTime()
{
    timeval cur_tm;
    gettimeofday(&cur_tm,NULL);
    return (long long)cur_tm.tv_sec*1000000 + cur_tm.tv_usec;
}

void SYS::cfgFileCheck(bool first)
{
    struct stat f_stat;
    if(stat(cfgFile().c_str(),&f_stat) != 0)
        return ;
    bool up = false;
    if(cfgTime != f_stat.st_mtime)
        up = true;

    cfgTime = f_stat.st_mtime;
    mess_info("cfgFileCheck","file check...");
    if(up && !first)
    {
        mess_info("cfgFileCheck","file modify...");
        modifClr();
        load();
    }
}

#define TABLE_SCADA_DATA_ID "create table if not exists scada_data_id("\
        "data_id integer primary key not null, "\
        "value_type integer, "\
        "data_src integer, "\
        "gain double, "\
        "shift double);"

int SYS::start()
{
    vector<string> lst;
    list(lst);

    for(unsigned i_a=0; i_a < lst.size(); i_a++)
    {
        try
        {
            at(lst[i_a]).at().subStart();
        }
        catch(TError err)
        {

        }
    }
    cfgFileCheck(true);

    string str = TABLE_SCADA_DATA_ID;

    sys->db().at().at("SQLite").at().open("Test");
    sys->db().at().at("SQLite").at().at("Test").at().setDBPath("./test.db");
    sys->db().at().at("SQLite").at().at("Test").at().enable();
    sys->db().at().at("SQLite").at().at("Test").at().sqlReq(str);

    mStopFlg = 0;
    unsigned int i_cnt = 1;
    vector<string> ls;
    while(!mStopFlg)
    {
        /* do something */
        /**** config file change periodic check */
        if(i_cnt % 2 == 0)
        {
            cfgFileCheck(false);
        }


        usleep(1000 * 1000); /* 1s */
        i_cnt++;
    }
    /* �ر�ʱ���� ���� */
    save();
    for(int i_a=lst.size()-1; i_a >= 0; i_a--)
    {
        try
        {
            at(lst[i_a]).at().subStop();
        }
        catch(TError err)
        {

        }
    }


    return mStopFlg;
}

void SYS::stop()
{
    mStopFlg = SIGUSR1;
}

void SYS::sighandler(int signal)
{
    switch(signal)
    {
    case SIGINT:
        sys->mStopFlg=signal;
        break;
    case SIGTERM:
        sys->mStopFlg=signal;
        break;
    case SIGFPE:
        exit(1);
        break;
    case SIGCHLD:
    {
        int status;
        pid_t pid = wait(&status);
        if(!WIFEXITED(status) && pid > 0)
            ;
        break;
    }
    case SIGPIPE:
        //mess_warning(SYS->nodePath().c_str(),_("Broken PIPE signal!"));
        break;
    case SIGSEGV:
        //mess_emerg(SYS->nodePath().c_str(),_("Segmentation fault signal!"));
        break;
    case SIGABRT:
        //mess_emerg(SYS->nodePath().c_str(),_("OpenSCADA is aborted!"));
        break;
    case SIGALRM:
        break;
    default:
        //mess_warning(SYS->nodePath().c_str(),_("Unknown signal %d!"),signal);
        break;
    }
}
