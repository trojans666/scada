#ifndef __MESS_H
#define __MESS_H


#include <stdio.h>
#include <string>
#include <vector>

using std::string;
using std::vector;


namespace SCADA
{

class Mess
{
public:
    enum Level
    {
       Fatal,Alert,Crit,Error,Warn,Notice,Info,Debug,Trace,Notset,Unknown
    };
    /* record */
    class SRec
    {
    public:
        SRec( ) : time(0), categ(""), level(Mess::Debug), mess("") { }
        SRec( time_t itm, int iutime, const string &icat, Level ilev, const string &imess ) :
            time(itm), utime(iutime), categ(icat), level(ilev), mess(imess) { }

        time_t	time;
        int	utime; /* 微秒 */
        string	categ;
        Level	level;
        string	mess;
    };

    Mess();
    ~Mess();

    void put(const char *categ,char level,const char *fmt,...);
    /** b_tm 起始时间 e_tm 结束时间 这个段内的数据 存放到 recs中 */
    void get(time_t b_tm,time_t e_tm,vector<Mess::SRec> &recs,const string &category="",char level=Debug);
};

}


#endif
