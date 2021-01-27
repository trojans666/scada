/**
  ¥´ ‰¿‡
*/
#ifndef __SUBTRANSPORT_H
#define __SUBTRANSPORT_H

#include "subsys.h"

#define SUBTRANSPORT_VER        1
#define SUBTRNASPORT_ID         "Transport"
#define SUBTRANSPORT_NAME       "Transports"

namespace SCADA
{

class SubTransport;
class ModTransport;

/* transport out */
class TransportOut : public CtrlNode
{
public:
    TransportOut(const string &id);
    virtual ~TransportOut();
    const string &id() {return mId;}

    string name();
    string addr() {return mAddr;}
    void setName(const string &inm) {mName = inm;}
    void setAddr(const string &addr) {mAddr = addr;}

    bool startStat() {return run_st;}

    virtual void start() {}
    virtual void stop() {}

    virtual int messIO(const char *obuf,int len_ob,char *ibuf=NULL,int len_ib=0,int time = 0, bool noRes = false)
    {return 0;}

    void messProtIO(const string &prot);

    ModTransport &owner();

protected:
    void load_() {}
    void save_() {}

    bool run_st;
private:
    const string &nodeName() {return mId;}
    string mId;
    string mName;
    string mAddr;
};

class ModTransport : public Module
{
public:
    ModTransport(const string &id);
    virtual ~ModTransport();

    /* output transport */
    void outList(vector<string> &list) {chldList(mOut,list);}
    bool outPresent(const string &name) {return chldPresent(mOut,name);}
    void outAdd(const string &name);
    void outDel(const string &name,bool complete = false) {chldDel(mOut,name,-1,complete);}
    AutoHD<TransportOut> outAt(const string &name) {return chldAt(mOut,name);}

    SubTransport &owner();
protected:
    virtual TransportOut *Out(const string &name)
    {
        throw TError(nodePath().c_str(),"Output transport no support!!");
    }
private:
    int mOut;
};

class SubTransport : public SubSys
{
public:
    SubTransport();
    ~SubTransport();

    int subVer() {return SUBTRANSPORT_VER;}

    void subStart();
    void subStop();

    void outTrList(vector<string> &ls);
    AutoHD<ModTransport> at(const string &iid) {return modAt(iid);}

protected:
    void load_() {}
    void save_() {}
private:
    int mMod;
};


}


#endif
