#include "log.h"
#include "bd_sqlite.h"

/* module info */
#define SQL_MOD_ID  "SQLite"
#define SQL_MOD_NAME    "DB SQLite"
#define SQL_MOD_TYPE    SUBDB_ID
#define SQL_VER_TYPE    SUBDB_VER
#define SQL_VERSION         "0.0.1"
#define SQL_AUTORS          "BigTao"
#define SQL_DESCRIPTION     "BD module. Provides support of the BD SQLite"
#define SQL_LICENSE         "GPL2"


BDSQLite::ModSqlite *BDSQLite::mod;

extern "C"
{
    Module::SAt module(int n_mod)
    {
        if(n_mod == 0)
            return Module::SAt(SQL_MOD_ID,SQL_MOD_TYPE,SQL_VER_TYPE);
        return Module::SAt("");
    }

    Module *attach(const Module::SAt &AtMod,const string &source)
    {
        if(AtMod == Module::SAt(SQL_MOD_ID,SQL_MOD_TYPE,SQL_VER_TYPE))
        {
            return new BDSQLite::ModSqlite(source);
        }
        return NULL;
    }
}

using namespace BDSQLite;

/* module database */
ModSqlite::ModSqlite(string name) : ModDB(SQL_MOD_ID)
{
    mod = this;
    mName = SQL_MOD_NAME;
    mType = SQL_MOD_TYPE;
    mVers = SQL_VERSION;
    mAutor = SQL_AUTORS;
    mDescr = SQL_DESCRIPTION;
    mSource = name;
}

ModSqlite::~ModSqlite()
{

}

DataBase *ModSqlite::openBD(const string &iid)
{
    return new DataBaseSqlite(iid);
}

string ModSqlite::sqlReqCode(const string &req,char symb)
{
    string sout = req;
    for(unsigned int i_sz = 0; i_sz < sout.size(); i_sz++)
    {
        if(sout[i_sz] == symb)
            sout.replace(i_sz++,1,2,symb);
    }
    return sout;
}

DataBaseSqlite::DataBaseSqlite(const string &iid):DataBase(iid),m_db(NULL)
{

}

DataBaseSqlite::~DataBaseSqlite()
{

}

void DataBaseSqlite::enable()
{
    if(m_db)
        return ; /*maybe isopen */
    int rc = sqlite3_open(path().c_str(),&m_db);
    if(rc)
    {
        string err = sqlite3_errmsg(m_db);
        sqlite3_close(m_db);
        throw TError(nodePath().c_str(),"Open DB failed!");
    }
    mess_info("enable","enable...\n");
    DataBase::enable();
}

bool DataBaseSqlite::isEnable()
{
    return (m_db != NULL) ? true : false;
}

void DataBaseSqlite::disable()
{
    DataBase::disable();
    /* close db */
    ResAlloc res(mRes,true);
    sqlite3_close(m_db);
    m_db = NULL;
}

DBTable *DataBaseSqlite::openTable(const string &inm,bool create)
{
    return new TableSqlite(inm,this,create);
}

void DataBaseSqlite::sqlReq(const string &ireq, vector< vector<string> > *tbl)
{
    char *zErrMsg = NULL;
    int rc, nrow = 0, ncol = 0;
    char **result = NULL;
    mess_info("sqlReq","sqlReq");
    if(tbl)
        tbl->clear();

    //> Commit set
    string req = ireq;
    ResAlloc res(mRes,true);

    //> Put request
    rc = sqlite3_get_table(m_db,req.c_str(),&result, &nrow, &ncol, &zErrMsg);
    if(rc != SQLITE_OK)
    {
        string err = "Unknown error";
        if(zErrMsg)
        {
            err = zErrMsg;
            sqlite3_free(zErrMsg);
        }
        //mess_err(nodePath().c_str(),_("Request error: %s"),req.c_str());
        throw TError(100+rc,nodePath().c_str(),"Getting table error: %s",err.c_str());
    }
    if(tbl && ncol > 0)
    {
        vector<string> row;
        //>> Add head
        for(int i = 0; i < ncol; i++)
            row.push_back(result[i]?result[i]:"");
        tbl->push_back(row);
        //>> Add data
        for(int i = 0; i < nrow; i++)
        {
            row.clear();
            for(int ii = 0; ii < ncol; ii++)
                row.push_back(result[(i+1)*ncol+ii]);
            tbl->push_back(row);
        }
    }
    if(result)
        sqlite3_free_table(result);
}

void DataBaseSqlite::transOpen()
{
    ResAlloc res(mRes,true);
    sqlReq("BEGIN;");
}

void DataBaseSqlite::transCommit()
{
    ResAlloc res(mRes,true);
    sqlReq("COMMIT;");
}

TableSqlite::TableSqlite(string inm,DataBaseSqlite *iown,bool create) : DBTable(inm)
{
    setNodePrev(iown);
    #if 0
    try
    {
        /*获取表结构 */
	string req = "SELECT * FROM '"+mod->sqlReqCode(name())+"' LIMIT 0;";	//!! Need for table present checking
	owner().sqlReq(req);
	req ="PRAGMA table_info('"+mod->sqlReqCode(name())+"');";
	owner().sqlReq(req, &tblStruct);
    }
    catch(...) { if( !create ) throw; }
    #endif // 0
}

TableSqlite::~TableSqlite()
{

}

DataBaseSqlite &TableSqlite::owner()
{
    return (DataBaseSqlite&)DBTable::owner();
}
