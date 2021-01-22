#ifndef __STROPT_H
#define __STROPT_H


#include <math.h>

#include <string>

using std::string;

#define vmin(a,b)   ((a) < (b) ? (a) : (b))
#define vmax(a,b)   ((a) > (b) ? (a) : (b))

namespace SCADA
{


class StrOpt
{
public:

    enum IntView
    {
        Dec, Oct,Hex
    };

    enum Code
    {
        PathEl,Custom,FormatPrint
    };


    /** 字符串操作函数 */
    static string strNoSpace(const string &val); /**清空所有空格 */
    static string strSepParse(const string &path, int level, char sep, int *off = NULL );
    static string strParse( const string &str, int level, const string &sep, int *off = NULL, bool mergeSepSymb = false );
    static string strLine( const string &str, int level, int *off = NULL );
    static string pathLev( const string &path, int level, bool encode = true, int *off = NULL );
    static string path2sepstr( const string &path, char sep = '.' );
    static string sepstr2path( const string &str, char sep = '.' );
    static string strEncode( const string &in, Code tp, const string &symb = " \t\n");
    static string strDecode( const string &in, Code tp = Custom );

    /**转换到string*/
    static string int2str( int val, IntView view = Dec );
    static string uint2str( unsigned val, IntView view = Dec );
    static string ll2str( long long val, IntView view = Dec );
    static string real2str( double val, int prec = 15, char tp = 'g' );
    /** 实数转换 */
    static double realRound( double val, int dig = 0, bool toint = false )
    {
        double rez = floor(val*pow(10,dig)+0.5)/pow(10,dig);
        if( toint ) return floor(rez+0.5);
        return rez;
    }
    /** 地址转换 */
    static string addr2str( void *addr );
    static void *str2addr( const string &str );

};

}

#endif // __STROPT_H
