#include "log.h"
#include "subtransport.h"

using namespace SCADA;

SubTransport::SubTransport() : SubSys(SUBTRNASPORT_ID,SUBTRANSPORT_NAME,true)
{

}

SubTransport::~SubTransport()
{}

void SubTransport::outTrList(vector<std::string> &ls)
{
    ls.clear();

    vector<string> t_ls, m_ls;
    modList(t_ls);
    for( int i_tp = 0; i_tp < t_ls.size(); i_tp++ )
    {
        at(t_ls[i_tp]).at().outList(m_ls);
        for( int i_t = 0; i_t < m_ls.size(); i_t++ )
            ls.push_back(t_ls[i_tp]+"."+m_ls[i_t]);
    }
}

void SubTransport::subStart()
{
    mess_info(nodePath().c_str(),"Start subsystem.");

    vector<string> t_lst, o_lst;
    modList(t_lst);
    for( int i_t = 0; i_t < t_lst.size(); i_t++ )
    {
        AutoHD<ModTransport> mod = modAt(t_lst[i_t]);

        o_lst.clear();
        mod.at().outList(o_lst);
        for( int i_o = 0; i_o < o_lst.size(); i_o++ )
            try
        {
            AutoHD<TransportOut> out = mod.at().outAt(o_lst[i_o]);
            if( !out.at().startStat())
                out.at().start();
        }catch( TError err )
        {
            mess_err(err.cat.c_str(),"%s",err.mess.c_str());
            mess_err(nodePath().c_str(),"Start output transport <%s> error.",o_lst[i_o].c_str());
        }
    }

    //> Controllers start
    SubSys::subStart( );
}

void SubTransport::subStop()
{
    mess_info(nodePath().c_str(),"Stop subsystem.");

    vector<string> t_lst, o_lst;
    modList(t_lst);
    for( int i_t = 0; i_t < t_lst.size(); i_t++ )
    {
        AutoHD<ModTransport> mod = modAt(t_lst[i_t]);

        o_lst.clear();
        mod.at().outList(o_lst);
        for( int i_o = 0; i_o < o_lst.size(); i_o++ )
            try
        {
            AutoHD<TransportOut> out = mod.at().outAt(o_lst[i_o]);
            if( out.at().startStat() ) out.at().stop();
        }catch( TError err )
        {
            mess_err(err.cat.c_str(),"%s",err.mess.c_str());
            mess_err(nodePath().c_str(),"Stop output transport <%s> error.",o_lst[i_o].c_str());
        }
    }

    SubSys::subStop( );
}

/*** mod transport */
ModTransport::ModTransport(const std::string &id) : Module(id)
{
    mOut = grpAdd("out_");
}

ModTransport::~ModTransport()
{
    nodeDelAll();
}

SubTransport &ModTransport::owner()
{
    return (SubTransport&)Module::owner();
}

void ModTransport::outAdd(const std::string &name)
{
    if(chldPresent(mOut,name))
        return ;
    chldAdd(mOut,Out(name));
}

/**  transport out */
TransportOut::TransportOut(const std::string &id)
{
    mId = id;
}

TransportOut::~TransportOut()
{

}

ModTransport &TransportOut::owner()
{
    return *(ModTransport*)nodePrev();
}

void TransportOut::messProtIO(const std::string &prot)
{
//    if( !sys->protocol().at().modPresent(prot) )
//    throw TError(nodePath().c_str(),"Transport protocol '%s' no present",prot.c_str());
//    sys->protocol().at().at(prot).at().outMess( io, *this );
}
