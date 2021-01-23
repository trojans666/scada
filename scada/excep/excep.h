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

    static string getStack(); /*��ȡ��ջ����*/
    static string getErrno(); /*��ȡerrno����*/

    int cod;
    string cat;
    string mess;
};


#endif // __EXCEP_H
