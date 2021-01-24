#include <stdio.h>
#include <stdlib.h>

#include "stropt.h"

using namespace SCADA;

/**
删除开头和结尾的空格
*/
string StrOpt::strNoSpace(const string &val)
{
    int beg = -1, end = -1;

    for(unsigned int i_s = 0; i_s < val.size(); i_s++)
        if(val[i_s] != ' ' && val[i_s] != '\n' && val[i_s] != '\t')
        {
            if(beg < 0)
                beg = i_s;
            end = i_s;
        }

    return (beg>=0) ? val.substr(beg,end-beg+1) : "";
}

/**
对path进行分割，分隔符是sep,level控制是第几个0.1.2.3 off是偏移，表示从path的第几个字符开始解析
eg.
    int off = 1;
   std::cout << TSYS::strSepParse("122.34.56.78",0,'.',&off);
   ->22
   off=0
   ->123
*/
string StrOpt::strSepParse( const string &path, int level, char sep, int *off)
{
    unsigned int an_dir = off ? *off : 0;
    int t_lev = 0;
    unsigned int t_dir;

    if( an_dir >= path.size() )
        return "";
    while(true)
    {
        t_dir = path.find(sep,an_dir);
        if( t_dir == string::npos )
        {
            if( off )
                *off = path.size();
            return (t_lev == level) ? path.substr(an_dir) : "";
        }
        else if( t_lev == level )
        {
            if( off )
                *off = t_dir+1;
            return path.substr(an_dir,t_dir-an_dir);
        }
        an_dir = t_dir+1;
        t_lev++;
    }
    return "";
}

/**
strSepParse的扩展版 sep可以是字符串分割了 merge允许合并
*/
string StrOpt::strParse( const string &path, int level, const string &sep, int *off, bool mergeSepSymb)
{
    unsigned int an_dir = off ? *off : 0;
    int t_lev = 0;
    unsigned int t_dir;

    if( an_dir >= path.size() || sep.empty() )
        return "";
    while(true)
    {
        t_dir = path.find(sep,an_dir);
        if( t_dir == string::npos )
        {
            if( off )
                *off = path.size();
            return (t_lev == level) ? path.substr(an_dir) : "";
        }
        else if( t_lev == level )
        {
            if( off )
                *off = t_dir+sep.size();
            return path.substr(an_dir,t_dir-an_dir);
        }
        if( mergeSepSymb && sep.size() == 1 )
            for( an_dir = t_dir; an_dir < path.size() && path[an_dir] == sep[0]; )
                an_dir++;
        else
            an_dir = t_dir+sep.size();
        t_lev++;
    }
    return "";
}

/**
获取一行 相当于对str中找\r\n
*/
string StrOpt::strLine( const string &str, int level, int *off)
{
    int an_dir = off ? *off : 0;
    int t_lev = 0, edLnSmbSz = 1;
    size_t t_dir;

    if(an_dir >= (int)str.size())
        return "";
    while(true)
    {
        for(t_dir = an_dir; t_dir < str.size(); t_dir++)
            if(str[t_dir] == '\x0D' || str[t_dir] == '\x0A')
            {
                edLnSmbSz = (str[t_dir] == '\x0D' && ((t_dir+1) < str.size()) && str[t_dir+1] == '\x0A') ? 2 : 1;
                break;
            }
        if(t_dir >= str.size())
        {
            if(off)
                *off = str.size();
            return (t_lev==level) ? str.substr(an_dir) : "";
        }
        else if(t_lev == level)
        {
            if(off)
                *off = t_dir+edLnSmbSz;
            return str.substr(an_dir,t_dir-an_dir);
        }
        an_dir = t_dir+edLnSmbSz;
        t_lev++;
    }
    return "";
}

/**
对path路径分割 /path1/path2/path3 以'/'分割
*/
string StrOpt::pathLev( const string &path, int level, bool encode, int *off)
{
    unsigned int an_dir = off ? *off : 0;
    int t_lev = 0;
    unsigned int t_dir;

    //- First separators pass -
    while( an_dir<path.size() && path[an_dir]=='/' )
        an_dir++;
    if( an_dir >= path.size() )
        return "";
    //- Path level process -
    while(true)
    {
        t_dir = path.find("/",an_dir);
        if( t_dir == string::npos )
        {
            if( off )
                *off = path.size();
            return (t_lev == level) ? ( encode ? StrOpt::strDecode(path.substr(an_dir),StrOpt::PathEl) : path.substr(an_dir) ) : "";
        }
        else if( t_lev == level )
        {
            if( off )
                *off = t_dir;
            return encode ? StrOpt::strDecode(path.substr(an_dir,t_dir-an_dir),StrOpt::PathEl) : path.substr(an_dir,t_dir-an_dir);
        }
        an_dir = t_dir;
        t_lev++;
        while( an_dir<path.size() && path[an_dir]=='/' )
            an_dir++;
    }
}

/**
 将path转换为sep分割的字符串 例如 /xx1/xx2/xx3 sep='.' -> xx1.xx2.xx3
*/
string StrOpt::path2sepstr( const string &path, char sep)
{
    string rez, curv;
    int off = 0;
    while( !(curv=StrOpt::pathLev(path,0,false,&off)).empty() )
        rez+=curv+sep;
    if(!rez.empty())
        rez.resize(rez.size()-1);

    return rez;
}

/**
将str转换成path格式
*/
string StrOpt::sepstr2path( const string &str, char sep)
{
    string rez, curv;
    int off = 0;
    while( !(curv=StrOpt::strSepParse(str,0,sep,&off)).empty() )
        rez+="/"+curv;

    return rez;
}
string StrOpt::strEncode( const string &in, StrOpt::Code tp, const string &symb)
{
    unsigned int i_sz;
    string sout;

    switch(tp)
    {
    case StrOpt::PathEl:
        sout = in;
        for( i_sz = 0; i_sz < sout.size(); i_sz++ )
            switch( sout[i_sz] )
            {
            case '/':
                sout.replace(i_sz,1,"%2f");
                i_sz+=2;
                break;
            case '%':
                sout.replace(i_sz,1,"%25");
                i_sz+=2;
                break;
            }
        break;
    case StrOpt::Custom:
        sout.reserve(in.size()+10);
        for( i_sz = 0; i_sz < in.size(); i_sz++ )
        {
            unsigned int i_smb;
            for( i_smb = 0; i_smb < symb.size(); i_smb++ )
                if( in[i_sz] == symb[i_smb] )
                {
                    char buf[4];
                    sprintf(buf,"%%%02X",(unsigned char)in[i_sz]);
                    sout += buf;
                    break;
                }
            if( i_smb >= symb.size() )
                sout+=in[i_sz];
        }
        break;
    case StrOpt::FormatPrint:
        sout = in;
        for( i_sz = 0; i_sz < sout.size(); i_sz++ )
            if( sout[i_sz] == '%' )
            {
                sout.replace(i_sz,1,"%%");
                i_sz++;
            }
        break;
    }
    return sout;
}
string StrOpt::strDecode( const string &in, StrOpt::Code tp)
{
    unsigned int i_sz;
    string sout("");

    switch(tp)
    {
    case StrOpt::PathEl:
    case StrOpt::Custom:
        sout.reserve(in.size());
        for( i_sz = 0; i_sz < in.size(); i_sz++ )
            switch( in[i_sz] )
            {
            case '%':
                if( i_sz+2 < in.size() )
                {
                    sout+=(char)strtol(in.substr(i_sz+1,2).c_str(),NULL,16);
                    i_sz+=2;
                }
                else
                    sout+=in[i_sz];
                break;
            default:
                sout+=in[i_sz];
            }
        break;

    default:
        break;
    }

    return sout;
}

/**转换到string*/
string StrOpt::int2str( int val, IntView view)
{
    char buf[256];
    if(view == StrOpt::Dec)
        snprintf(buf,sizeof(buf),"%d",val);
    else if(view == StrOpt::Oct)
        snprintf(buf,sizeof(buf),"%o",val);
    else if(view == StrOpt::Hex)
        snprintf(buf,sizeof(buf),"%x",val);

    return buf;
}
string StrOpt::uint2str( unsigned val, IntView view)
{
    char buf[256];
    if(view == StrOpt::Dec)
        snprintf(buf,sizeof(buf),"%u",val);
    else if(view == StrOpt::Oct)
        snprintf(buf,sizeof(buf),"%o",val);
    else if(view == StrOpt::Hex)
        snprintf(buf,sizeof(buf),"%x",val);

    return buf;
}
string StrOpt::ll2str( long long val, IntView view)
{
    char buf[256];
    if(view == StrOpt::Dec)
        snprintf(buf,sizeof(buf),"%lld",val);
    else if(view == StrOpt::Oct)
        snprintf(buf,sizeof(buf),"%llo",val);
    else if(view == StrOpt::Hex)
        snprintf(buf,sizeof(buf),"%llx",val);

    return buf;
}
string StrOpt::real2str( double val, int prec, char tp)
{
    char buf[256];
    if(tp == 'g')
        snprintf(buf,sizeof(buf),"%.*g",prec,val);
    else if(tp == 'e')
        snprintf(buf,sizeof(buf),"%.*e",prec,val);
    else
        snprintf(buf,sizeof(buf),"%.*f",prec,val);

    return buf;
}

/** 地址转换 */
string StrOpt::addr2str( void *addr )
{
    char buf[sizeof(void*)*2+3];
    snprintf(buf,sizeof(buf),"%p",addr);

    return buf;
}
void * StrOpt::str2addr( const string &str )
{
    return (void *)strtoul(str.c_str(),NULL,16);
}

