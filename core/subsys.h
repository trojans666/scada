/**
  ��ϵͳ��
*/

#ifndef __SUBSYS_H
#define __SUBSYS_H

#include <string>
#include <vector>

#include "module.h"

using std::string;
using std::vector;

namespace SCADA
{


class ModSchedul;
class SYS;
class Module;

class SubSys : public CtrlNode
{
public:
    SubSys(const char *id,const char *name,bool modi = false); /** modi �Ƿ񴴽�module group */
    virtual ~SubSys();

    const string &subId() {return mId;}
    string subName();
    virtual int subVer() {return 0;} /*��ϵͳ�汾 */

    bool subModule() {return mSubModule;} /*�Ƿ���module */

    virtual void subStart();
    virtual void subStop();

    /* modules */
    void modList(vector<string> &list);
    bool modPresent(const string &name);
    void modAdd(Module *mod);
    void modDel(const string &name);
    AutoHD<Module> modAt(const string &name);

    SYS &owner();
    /* ����ִ�� */
    virtual void perCall(unsigned int timeout);
protected:
    bool mStart;
private:
    /* �ڵ����� */
    const string &nodeName() {return subId();}
    bool mSubModule; /*�Ƿ���module */
    int mMod; /*grp id */
    string mId;
    string mName;
};

}

#endif
