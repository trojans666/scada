
#include "subsys.h"

SubSys::SubSys(const char *id, const char *name, bool modi)
    :mStart(false)
    ,mSubModule(modi)
    ,mMod(-1)
    ,mId(id)
    ,mName(name)
{
    if(subModule())
        mMod = grpAdd("mod_"); /*create module group */
}

SubSys::~SubSys()
{
    nodeDelAll();
}

SYS & SubSys::owner()
{
    return  *((SYS*)nodePrev());
}

string SubSys::subName()
{
    return mName.size() ? mName : mId;
}

void SubSys::modList(vector<std::string> &list)
{
    if(!subModule())
        throw TError(nodePath().c_str(),"The subsystem is not modular!");
    chldList(mMod,list);
}


bool SubSys::modPresent( const string &name )
{
    if( !subModule() ) throw TError(nodePath().c_str(),"The subsystem is not modular!");
    return chldPresent(mMod,name);
}

void SubSys::modAdd( Module *modul )
{
    if( !subModule() )
        throw TError(nodePath().c_str(),"The subsystem is not modular!");
    if( chldPresent(mMod,modul->modId()) ) return;
    chldAdd(mMod,modul);
}

void SubSys::modDel( const string &name )
{
    if( !subModule() )
        throw TError(nodePath().c_str(),"The subsystem is not modular!");
    //mess_info(nodePath().c_str(),"Disconnect module <%s>!",name.c_str());
    chldDel(mMod,name);
}

AutoHD<Module> SubSys::modAt( const string &name )
{
    if( !subModule() ) throw TError(nodePath().c_str(),"The subsystem is not modular!");
    return chldAt(mMod,name);
}

void SubSys::subStart( )
{
    if( !subModule() )
        return;

    vector<string> list;
    modList(list);
    for( unsigned i_m=0; i_m < list.size(); i_m++ )
    try
    {
        modAt(list[i_m]).at().modStart( );
    }
    catch(TError err)
    {
       // mess_err(err.getCat().c_str(),"%s",err.getMsg().c_str());
       // mess_err(nodePath().c_str(),"Start module '%s' error.",list[i_m].c_str());
    }

    mStart = true;
}

void SubSys::subStop( )
{
    if( !subModule() )	return;
    vector<string> list;
    modList(list);
    for(unsigned i_m=0; i_m < list.size(); i_m++)
    try{ modAt(list[i_m]).at().modStop( ); }
    catch(TError err)
    {
       // mess_err(err.getCat().c_str(),"%s",err.getMsg().c_str());
       // mess_err(nodePath().c_str(),"Stop module '%s' error.",list[i_m].c_str());
    }

    mStart = false;
}

void SubSys::perCall(unsigned int timeout)
{
    if(!subModule())
        return ;
    vector<string> list;
    modList(list);

    for(unsigned int i_m = 0;i_m < list.size();i_m++)
    {
        try
        {
            modAt(list[i_m]).at().perCall(timeout);
        }
        catch(TError &err)
        {

        }
    }
}
