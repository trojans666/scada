#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

#define mess_debug(cat,fmt,args...) mLog->put(cat,Log::Debug,fmt,##args)
#define mess_info(cat,fmt,args...) mLog->put(cat,Log::Info,fmt,##args)
#define mess_note(cat,fmt,args...) mLog->put(cat,Log::Notice,fmt,##args)
#define mess_warning(cat,fmt,args...) mLog->put(cat,Log::Warning,fmt,##args)
#define mess_err(cat,fmt,args...) mLog->put(cat,Log::Error,fmt,##args)
#define mess_crit(cat,fmt,args...) mLog->put(cat,Log::Crit,fmt,##args)
#define mess_alert(cat,fmt,args...) mLog->put(cat,Log::Alert,fmt,##args)
#define mess_emerg(cat,fmt,args...) mLog->put(cat,Log::Emerg,fmt,##args)

class Log
{
public:
    enum Type
    {
        Debug,Info,Notice,Warning,Error,Crit,Alert,Emerg
    };

    enum Dir
    {
        DIR_SYSLOG = 1 << 0,
        DIR_STDOUT = 1 << 1,
        DIR_STDERR = 1 << 2,
        DIR_ARCHIVE = 1 << 3
    };

    Log();
    ~Log();

    void load();
    void save();

    int logDirect() {return mLogDir;}
    int messLevel() {return mMessLevel;}

    void setLogDirect(int dir);
    void setMessLevel(int level);

    void put(const char *cat, char level,const char *fmt,...);
private:
    unsigned char mMessLevel;
    unsigned char mLogDir;
    Log(Log &other) {}
    Log &operator=(Log &other) {return *this;}
};

extern Log *mLog;

#endif // __LOG_H
