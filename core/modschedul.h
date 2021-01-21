/**
 ������
*/

#ifndef __MODSCHEDUL_H
#define __MODSCHEDUL_H

#include <string>
#include <vector>

#include "subsys.h"
#include "resalloc.h"

using std::string;
using std::vector;

#define MODSCHEDUL_ID       "ModSched"
#define MODSCHEDUL_NAME     "Modules sheduler"

class ModSchedul : public SubSys
{
public:
    class SHD
    {
    public:
        SHD():hd(NULL),tm(0) {}
        SHD(void *ihd,time_t itm,const string &iname):hd(ihd),tm(itm),name(iname) {}

        void *hd; /*�������� ����δattach */
        vector<string> use; /* ��������˹��������ʾʹ������Щģ�� */
        time_t tm;  /*   �Թ��������ݽ����޸ģ��Զ����� */
        string name; /* �����·�� */
        string err; /* ��ģ��Ĵ�����Ϣ */
    };

    ModSchedul();

    ResRW &nodeRes()    {return mRes;}

    /** ��ȡlib */
    SHD lib(const string &name);
    int loadLibS();

    void libList(vector<string> &list);
    int libLoad(const string &iname, bool full);
    void libAtt(const string &name,bool full = false);
    void libDet(const string &name);

protected:
    void load_();
    void save_();
private:
    void scanDir(const string &paths,vector<string> &files); /* ɨ�蹲���·��*/
    bool checkFile(const string &name); /*��鹲��� */
    int libRegister(const string &name); /*ע�Ṳ���*/
    void libUnRegister(const string &name); /*ע������� */
    bool chkAllowMod(const string &name); /*����Ƿ������ļ����� */

    vector<SHD> mSchHD;
    ResRW mRes;
};


#endif
