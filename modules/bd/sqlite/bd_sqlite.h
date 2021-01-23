#ifndef __BD_SQLITE_H
#define __BD_SQLITE_H

#include <string>
#include <vector>
#include "module.h"
#include "subdb.h"
#include "resalloc.h"

#include "sqlite3.h"

using namespace SCADA;

namespace BDSQLite
{


class DataBaseSqlite;

class TableSqlite : public DBTable
{
public:
    TableSqlite(string name,DataBaseSqlite *bd,bool create);
    ~TableSqlite();

    DataBaseSqlite &owner();
private:
    vector< vector<string> > tblStruct;
};

/* database sqlite3 */
class DataBaseSqlite : public DataBase
{
public:
    DataBaseSqlite(const string &iid);
    ~DataBaseSqlite();

    void enable();
    bool isEnable();
    void disable();
    /* intoTran 是否开启事务 */
    void sqlReq(const string &req,vector< vector<string> > *tbl = NULL);

    void transOpen();
    void transCommit();

protected:

private:
    DBTable *openTable(const string &name,bool create);
    sqlite3 *m_db;
    ResRW mRes;
};

/* module */
class ModSqlite : public ModDB
{
public:
    ModSqlite(string name);
    ~ModSqlite();
    static string sqlReqCode(const string &req,char symb = '\'');
protected:
    void load_() {}
private:
    DataBase *openBD(const string &iid);
};

extern ModSqlite *mod;

}

#endif // __BD_SQLITE_H
