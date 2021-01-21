/**
    模块类
*/

#ifndef __MODULE_H
#define __MODULE_H

#include <string>
#include <vector>

#include "ctrlnode.h"
#include "subsys.h"

using std::string;
using std::vector;

class SubSys;

class Module : public CtrlNode
{
public:
    /** SAT attach module struct */
    class SAt
    {
    public:
        //Methods
        SAt( )	{ }
        SAt( const string &iid, const string &itype = "", int itver = 0 ) :
            id(iid), type(itype), t_ver(itver)	{ }

        bool operator==( const Module::SAt &amst ) const
        {
            if( amst.id == id && amst.type == type && amst.t_ver == t_ver ) return true;
            return false;
        }

        //Attributes
        string id;          //id module  模块id
        string type;        //Type module 模块类型
        int    t_ver;       //Type version module  模块类型版本
    };

    /** exp func */
    class ExpFunc
    {
    public:
        ExpFunc( const string &iprot, const string &idscr, void (Module::*iptr)() ) :
            prot(iprot), dscr(idscr), ptr(iptr)	{ }
        string prot;		//Prototip
        string dscr;		//Description
        void (Module::*ptr)();	//Adress)
    };

    Module(const string &id);
    virtual ~Module();

    /** 模块 id */
    const string &modId() {return mId;}
    string modName(); /**模块名称*/

    virtual void modStart() {}
    virtual void modStop() {}

    virtual void modInfo(vector<string> &list);
    virtual string modInfo(const string &name);

    /* export functions */
    void modFuncList(vector<string> &list);
    bool modFuncPresent(const string &prot);
    ExpFunc &modFunc(const string &prot);
    void modFunc(const string &prot,void (Module::**offptr)());

    SubSys &owner();

    virtual void perCall(unsigned int timeout) {}

protected:
    /* register export function */
    void modFuncReg(ExpFunc *func){mEfunc.push_back(func);}

    string mName;				// Name
    string mDescr;				// Describe
    string mType;				// Type
    string mVers;				// Version
    string mAutor;				// Autor
    string mLicense;			// License
    string mSource;				// Source of module (SO, in build, ....)


private:
    const string &nodeName() {return modId();}

    string mId;
    vector<ExpFunc *> mEfunc; /* export function list */
    static const char *l_info[]; /* list of the present info options */
};


#endif
