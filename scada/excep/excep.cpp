#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#endif // __linux__

#include "excep.h"

TError::TError()
    :cod(0)
    ,cat("")
    ,mess("")
{

}

TError::TError(const char *icat, const char *fmt,...)
{
    char str[256]; /*action!!! */
    va_list argptr;

    va_start(argptr,fmt);
    vsnprintf(str,sizeof(str),fmt,argptr);
    va_end(argptr);

    cod = 0;
    cat = icat;
    mess = str;
}

TError::TError(int icod, const char *icat, const char *fmt,...)
{
    char str[256];
    va_list argptr;

    va_start(argptr,fmt);
    vsnprintf(str,sizeof(str),fmt,argptr);
    va_end(argptr);

    cod = icod;
    cat = icat;
    mess = str;
}


string TError::getStack()
{
#ifdef __linux__
#define BFSIZE 255
    void *buffer[BFSIZE];
    int n = backtrace(buffer, BFSIZE);
    char **res = backtrace_symbols(buffer, n);
    std::string tmp;
    for (int i = 0; i < n; ++i)
    {
        std::string x = res[i];
        std::string plus;
        std::string addr;
        size_t pos = x.find("(");
        if (pos != std::string::npos)
        {
            x = x.substr(pos + 1); // skip executable name

            pos = x.find(")");
            if (pos != std::string::npos)
            {
                addr = x.substr(pos + 1);
                x = x.substr(0, pos);
            }

            pos = x.find("+");
            if (pos != std::string::npos)
            {
                plus = x.substr(pos);
                x = x.substr(0, pos);
            }
        }
        char zz[1000];
        {
            size_t sz = 1000;
            int status = 0;
            abi::__cxa_demangle( x.c_str(), zz, &sz, &status);

            if (!status)
            {
                tmp += zz;
                tmp += plus;
                tmp += addr;
            }
            else
            {
                tmp += res[i];
            }
            tmp += "\n";
        }
        // dladdr() test
        if (0)
        {
            Dl_info info;
            int n = dladdr(buffer[i], &info);
            if (!n)
                ;//printf("%d: dladdr() failed\n", i);
            else
            {
                size_t sz = 1000;
                int status = 0;
                abi::__cxa_demangle( info.dli_sname, zz, &sz, &status);

                //printf("%d: %s: %s\n", i, info.dli_fname, info.dli_sname);
                //if (!status)
                //printf("		%s\n", zz);
            }
        } // end of dladdr() test
    } // for (i)
    free(res);
    return tmp;
#else
    return "null";
#endif
}
/**
get errno string
*/
string TError::getErrno()
{
    string str("");
    str = strerror(errno);
    return str;
}

