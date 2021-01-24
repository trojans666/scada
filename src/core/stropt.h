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


    /** �ַ����������� */
    static string strNoSpace(const string &val); /**������пո� */
    static string strSepParse(const string &path, int level, char sep, int *off = NULL );
    static string strParse( const string &str, int level, const string &sep, int *off = NULL, bool mergeSepSymb = false );
    static string strLine( const string &str, int level, int *off = NULL );
    static string pathLev( const string &path, int level, bool encode = true, int *off = NULL );
    static string path2sepstr( const string &path, char sep = '.' );
    static string sepstr2path( const string &str, char sep = '.' );
    static string strEncode( const string &in, Code tp, const string &symb = " \t\n");
    static string strDecode( const string &in, Code tp = Custom );

    /**ת����string*/
    static string int2str( int val, IntView view = Dec );
    static string uint2str( unsigned val, IntView view = Dec );
    static string ll2str( long long val, IntView view = Dec );
    static string real2str( double val, int prec = 15, char tp = 'g' );
    /** ʵ��ת�� */
    static double realRound( double val, int dig = 0, bool toint = false )
    {
        double rez = floor(val*pow(10,dig)+0.5)/pow(10,dig);
        if( toint ) return floor(rez+0.5);
        return rez;
    }
    /** ��ַת�� */
    static string addr2str( void *addr );
    static void *str2addr( const string &str );

};

}

#endif // __STROPT_H
