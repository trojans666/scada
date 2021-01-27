#ifndef __SUBPROTOCOL_H
#define __SUBPROTOCOL_H

#include "subsys.h"

#define SUBPROTOCOL_VER     1
#define SUBPROTOCOL_ID
#define SUBPROTOCOL_NAME

namespace SCADA
{

class TransportOut;

class ModProtocol : public Module
{
public:
    ModProtocol();
    virtual ~ModProtocol();

    /* output protocol */
    virtual void outMess(TransportOut &tro)
    {
        throw TError(nodePath().c_str(),"Function <%s> no support!","outMess");
    }

protected:

private:

};

class SubProtocol : public SubSys
{
public:
    SubProtocol();
    ~SubProtocol();

    int subVer()    {return SUBPROTOCOL_VER;}

    AutoHD<ModProtocol> at(const string &iid) {return modAt(iid);}
protected:
    void load_() {}
    void save_() {}
private:

};

}

#endif
