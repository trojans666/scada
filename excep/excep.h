#ifndef __EXCEP_H
#define __EXCEP_H

#include <string>

using std::string;

class TError
{
public:
    TError();
    TError(const char *icat,const char *fmt,...);
    TError(int icod,const char *icat,const char *fmt,...);

    static string getStack(); /*获取堆栈数据*/
    static string getErrno(); /*获取errno数据*/

    int cod;
    string cat;
    string mess;
};


#endif // __EXCEP_H
