/**
���ݿ���
*/
#ifndef __SUBDB_H
#define __SUBDB_H

#include <string>
#include <vector>


#include "subsys.h"
#include "resalloc.h"
#include "autohd.h"
#include "ctrlnode.h"

using std::string;
using std::vector;

#define SUBDB_VER   1

#define SUBDB_ID    "DB"
#define SUBDB_NAME  "Data Bases"

namespace SCADA
{

class DataBase;
class ModDB;
class SubDB;

/* ����ĳ�����ݿ��еı� */
class DBTable : public CtrlNode
{
public:
    DBTable(const string &name);
    virtual ~DBTable();

    const string &name() {return mName;}
    DataBase &owner();
private:
    const string &nodeName() {return mName;}
    string mName; /* ���� */
};

/* ����һ�����ݿ� */
class DataBase : public CtrlNode
{
public:
    DataBase(const string &iid);
    virtual ~DataBase();
    /* ���ݿ�Ĵ򿪺͹ر� */
    virtual void enable() {}
    virtual void disable() {}

    void list(vector<string> &list) {chldList(mTbl,list);}
    bool openStat(const string &table) {return chldPresent(mTbl,table);}
    /* �˴���open�ǽ� table���뵽grp��û�еĻ����Դ�����create���� */
    void open(const string &table,bool create);
    void close(const string &table,bool del = false) {chldDel(mTbl,table,-1,del);}

    AutoHD<DBTable> at(const string &name) {return chldAt(mTbl,name);}

    /* sql request */
    virtual void sqlReq(const string &req,vector< vector<string> > *tbl = NULL)
    {
        throw TError(nodePath().c_str(),"Function <%s> no support!","sqlReq");
    }

    ModDB &owner();

    void setName(const string &vl) {mName = vl;}
    string name();
    /* ���ݿ��·�� */
    void setDBPath(const string &path) {mPath = path;}
    string path() {return mPath;}

protected:
    virtual DBTable *openTable(const string &table,bool create)
    {
        throw TError(nodePath().c_str(),"Function <%s> no support!","openTable");
    }
    void load_() {}
    void save_() {}

    AutoHD<CtrlNode> chldAt(char igr,const string &name,const string &user = "");
private:
    const string &nodeName() {return mId;}
    int mTbl;
    string mId;
    string mName;
    string mPath;
};
/* �������һ�����ݿ����� �� mysql sqlite3 */
class ModDB : public Module
{
public:
    ModDB(const string &id);
    virtual ~ModDB();

    /* open data base */
    void list(vector<string> &list) {chldList(m_db,list);}
    /* �ñ��Ƿ���� */
    bool openStat(const string &idb) {return chldPresent(m_db,idb);}
    /* �����open�ǰ� sqlite mysql ����module���ص�grp��  */
    void open(const string &iid);
    void close(const string &iid,bool er = false) {chldDel(m_db,iid,-1,er);}
    /* �������ݿ� */
    AutoHD<DataBase> at(const string &name) {return chldAt(m_db,name);}

    SubDB &owner();

protected:

private:
    const string &nodeName() {return mId;}
    virtual DataBase *openBD(const string &id) {throw TError(nodePath().c_str(),"Function <%s> no support!","openBD");}
    int m_db;
    string mId;

};

class SubDB : public SubSys
{
public:
    SubDB();
    ~SubDB();

    int subVer() {return SUBDB_VER;}

    void dbList(vector<string> &ls);

    /* open/close table */
    /* ��һ�����ݿ�ı� bdnΪ����·�� */
    AutoHD<DBTable> open(const string &bdn,bool create = false);
    void close(const string &bdn,bool del = false);

    AutoHD<ModDB> at(const string &iid) {return modAt(iid);}

    ResRW &nodeRes() {return mRes;}
protected:
    void load_() {}
    void save_() {}

private:
    ResRW mRes;
};

}
#endif // __SUBDB_H
