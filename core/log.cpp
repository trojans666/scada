#include <stdarg.h>

#ifdef __linux__
#include <syslog.h>
#endif // __linux__

#include <iostream>

#include "stropt.h"
#include "log.h"

Log::Log():mMessLevel(0),mLogDir(DIR_STDOUT)
{
    openlog("",0,LOG_USER);
}

Log::~Log()
{
    closelog();
}


void Log::setMessLevel(int level)
{
    mMessLevel = vmax(0,vmin(5,level));
}

void Log::setLogDirect(int dir)
{
    mLogDir = dir;
}

void Log::put(const char *cat,char level,const char *fmt,...)
{
    char mess[1024];
    va_list argptr;

    va_start(argptr,fmt);
    vsnprintf(mess,sizeof(mess),fmt,argptr);
    va_end(argptr);

    level = vmin(Emerg,vmax(-Emerg,level));
    if( abs(level) < messLevel() )
        return;

    //long long ctm = SYS::curTime();
    string s_mess = StrOpt::int2str(level) + " [" + cat + "] " + mess;
    //std::cout << s_mess.c_str() <<std::endl;
    if( mLogDir & DIR_SYSLOG)
    {
        int level_sys;
        switch( abs(level) )
        {
        case Debug:
            level_sys = LOG_DEBUG;
            break;
        case Info:
            level_sys = LOG_INFO;
            break;
        case Notice:
            level_sys = LOG_NOTICE;
            break;
        case Warning:
            level_sys = LOG_WARNING;
            break;
        case Error:
            level_sys = LOG_ERR;
            break;
        case Crit:
            level_sys = LOG_CRIT;
            break;
        case Alert:
            level_sys = LOG_ALERT;
            break;
        case Emerg:
            level_sys = LOG_EMERG;
            break;
        default:
            level_sys = LOG_DEBUG;
        }
        syslog(level_sys,"%s",s_mess.c_str());
    }
    printf("## %d\n",mLogDir);
    if( mLogDir & DIR_STDOUT )
        printf("jfsklfjs1111\n");
        //fprintf(stdout,"%s \n",s_mess.c_str());
    if( mLogDir & DIR_STDERR)
        fprintf(stderr,"%s \n",s_mess.c_str());

   // if( (mLogDir & DIR_ARCHIVE) && sys->present("Archive") )
   //     sys->archive().at().messPut( ctm/1000000, ctm%1000000, categ, level, mess );
}

void Log::load()
{
    /* 解析配置文件 */
}

void Log::save()
{

}

