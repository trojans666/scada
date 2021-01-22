#include "sys.h"
#include "stropt.h"
#include "subdb.h"
#include "log.h"

using namespace SCADA;

SubDB::SubDB():SubSys(SUBDB_ID,SUBDB_NAME,true)
{

}

SubDB::~SubDB()
{

}

void SubDB::dbList(vector<string> &ls)
{
    vector<string> tdb_ls,db_ls;
    modList(tdb_ls);

    for(unsigned int i_tp = 0;i_tp < tdb_ls.size();i_tp++)
    {
        sys->db().at().at(tdb_ls[i_tp]).at().list(db_ls);
        for(unsigned int i_db = 0;i_db < db_ls.size();i_db++)
        {
            ls.push_back(tdb_ls[i_tp] + "." + db_ls[i_db]);
        }
    }
}

AutoHD<DBTable> SubDB::open(const string &bdn,bool create)
{
    AutoHD<DBTable> tbl;
    mess_info("open","sub db open");
    try
    {
        string bd_t = StrOpt::strSepParse(bdn,0,'.');
        string bd_n = StrOpt::strSepParse(bdn,1,'.');
        string bd_tbl = StrOpt::strSepParse(bdn,2,'.');

        if(! at(bd_t).at().at(bd_n).at().openStat(bd_tbl))
        {
            at(bd_t).at().at(bd_n).at().open(bd_tbl,create);
        }
        tbl = at(bd_t).at().at(bd_n).at().at(bd_tbl);
    }
    catch(TError err)
    {

    }
    return tbl;
}

void SubDB::close(const string &bdn,bool del)
{
    try
    {
        string bd_t = StrOpt::strSepParse(bdn,0,'.');
        string bd_n = StrOpt::strSepParse(bdn,1,'.');
        string bd_tbl = StrOpt::strSepParse(bdn,2,'.');

        if(at(bd_t).at().at(bd_n).at().openStat(bd_tbl) &&
           at(bd_t).at().at(bd_n).at().at(bd_tbl).at().nodeUse() == 0)
        {
            at(bd_t).at().at(bd_n).at().close(bd_tbl,del);
        }
    }
    catch(TError err)
    {

    }
}

/********** Mod DB ************/
ModDB::ModDB(const string &id) : Module(id)
{
    mess_info("ModDB","## %s",id.c_str());
    mId = id;
    m_db = grpAdd("db_");
}

ModDB::~ModDB()
{
    nodeDelAll();
}

void ModDB::open(const string &iid)
{
    mess_info("open","mod db open ");
    if(openStat(iid))
        return ;

    chldAdd(m_db,openBD(iid));
}

SubDB &ModDB::owner()
{
    return (SubDB&)ModDB::owner();
}

/********* data base ************/
DataBase::DataBase(const string &iid)
    :mName("")
{
    mId = iid;
    mTbl = grpAdd("tbl_");
}

DataBase::~DataBase()
{

}

string DataBase::name()
{
    return mName.size() ? mName : mId;
}

ModDB &DataBase::owner()
{
    return *(ModDB*)nodePrev();
}

void DataBase::open(const string &table,bool create)
{
    mess_info("open","data base open");
    if(chldPresent(mTbl,table))
        return ;

    chldAdd(mTbl,openTable(table,create));
}

AutoHD<CtrlNode> DataBase::chldAt(char igr,const string &name,const string &user)
{
    try
    {
        return CtrlNode::chldAt(igr,name,user);
    }
    catch(...)
    {
        if(igr == mTbl && openStat(name))
        {
            open(name,false);
            return CtrlNode::chldAt(igr,name,user);
        }
        else
            throw;
    }
}

/***** table ****/
DBTable::DBTable(const string &name) : mName(name)
{

}

DBTable::~DBTable()
{

}

DataBase &DBTable::owner()
{
    return *(DataBase *)nodePrev();
}
