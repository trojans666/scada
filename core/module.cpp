#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>

#include <string.h>
#ifdef __linux__
#include <libintl.h>
#include <dlfcn.h>
#endif
#include "sys.h"
#include "excep.h"
#include "subsys.h"
#include "module.h"
#include "log.h"

using namespace SCADA;

const char *Module::l_info[] =
{
    "Module","Name","Type","Source","Version","Author","Description","License"
};

Module::Module(const std::string &id):mId(id)
{
    mess_info("Module","Module id = %s",mId.c_str());
}

Module::~Module()
{
    /* clean export function list */
    for(unsigned int i = 0;i < mEfunc.size();i++)
    {
        delete mEfunc[i];
    }
}

string Module::modName()
{
    return mName;
}

SubSys &Module::owner()
{
    return *((SubSys*)nodePrev());
}


void Module::modFuncList( vector<string> &list )
{
    list.clear();
    for(unsigned int i=0; i < mEfunc.size(); i++)
        list.push_back(mEfunc[i]->prot);
}

bool Module::modFuncPresent( const string &prot )
{
    for(unsigned int i=0; i < mEfunc.size(); i++)
        if( mEfunc[i]->prot == prot )
            return true;
    return false;
}

Module::ExpFunc &Module::modFunc( const string &prot )
{
    for(unsigned int i=0; i < mEfunc.size(); i++)
        if( mEfunc[i]->prot == prot )
            return *mEfunc[i];

    throw TError(nodePath().c_str(),"Function <%s> is not present in the module!",prot.c_str());
}

void Module::modFunc( const string &prot, void (Module::**offptr)() )
{
    *offptr = modFunc(prot).ptr;
}

void Module::modInfo( vector<string> &list )
{
    for(unsigned int i_opt = 0; i_opt < sizeof(l_info)/sizeof(char *); i_opt++ )
    list.push_back( l_info[i_opt] );
}

string Module::modInfo( const string &name )
{
    string info;

    if(name == l_info[0])	info = mId;
    else if(name == l_info[1]) info = mName;
    else if(name == l_info[2]) info = mType;
    else if(name == l_info[3]) info = mSource;
    else if(name == l_info[4]) info = mVers;
    else if(name == l_info[5]) info = mAutor;
    else if(name == l_info[6]) info = mDescr;
    else if(name == l_info[7]) info = mLicense;

    return info;
}

