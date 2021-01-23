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

/************ eg ******************
#define TABLE_SCADA_DATA_ID "create table if not exists scada_data_id("\
        "data_id integer primary key not null, "\
        "value_type integer, "\
        "data_src integer, "\
        "gain double, "\
        "shift double);""
"
string str = TABLE_SCADA_DATA_ID;

sys->db().at().at("SQLite").at().open("Test");
sys->db().at().at("SQLite").at().at("Test").at().setDBPath("./test.db");
sys->db().at().at("SQLite").at().at("Test").at().enable();
sys->db().at().at("SQLite").at().at("Test").at().sqlReq(str);

    string dbName = "test.db";
    sys->db().at().openDB(dbName);

#define TABLE_SCADA_DATA_ID "create table if not exists scada_data_id("\
        "data_id integer primary key not null, "\
        "value_type integer, "\
        "data_src integer, "\
        "gain double, "\
        "shift double);"""

    string req = TABLE_SCADA_DATA_ID;

    sys->db().at().sqlReqeust(dbName,req);

    dbName = "test1";
    sys->db().at().openDB(dbName);

    sys->db().at().sqlReqeust(dbName,req);


*/

bool SubDB::openDB(const std::string dbName, bool create)
{
    string iid = StrOpt::strSepParse(dbName,0,'.');
    if(iid.empty())
        iid = dbName;
    mess_info("openDB","##%s",iid.c_str());
    if(!at("SQLite").at().openStat(iid) && create)
    {

        at("SQLite").at().open(iid);
    }
    else
    {
        mess_info("openDB","false....");
        return false;
    }

    at("SQLite").at().at(iid).at().setDBPath(sys->DBDir() + "/" + dbName);
    at("SQLite").at().at(iid).at().enable();

    return true;
}

bool SubDB::isOpen(const std::string dbName)
{
    string iid = StrOpt::strSepParse(dbName,0,'.');
    if(iid.empty())
        iid = dbName;

    if(at("SQLite").at().openStat(iid))
    {
        return at("SQLite").at().at(iid).at().isEnable();
    }
    return false;
}

void SubDB::closeDB(const std::string dbName)
{
    string iid = StrOpt::strSepParse(dbName,0,'.');
    if(iid.empty())
        iid = dbName;

    if(at("SQLite").at().openStat(iid))
    {
        at("SQLite").at().at(iid).at().disable();
    }
}

void SubDB::sqlReqeust(const string dbName,const std::string &req, vector<vector<std::string> > *tbl)
{
    string iid = StrOpt::strSepParse(dbName,0,'.');
    if(iid.empty())
        iid = dbName;

    /* 这里不判断是否使能了  */
    mess_info("sqlRequest","##%s",iid.c_str());
    at("SQLite").at().at(iid).at().sqlReq(req,tbl);
}
