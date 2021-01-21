/**
 调度类
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

        void *hd; /*共享库存在 但是未attach */
        vector<string> use; /* 如果附加了共享库来显示使用了哪些模块 */
        time_t tm;  /*   对共享库的数据进行修改，自动更新 */
        string name; /* 共享库路径 */
        string err; /* 损坏模块的错误信息 */
    };

    ModSchedul();

    ResRW &nodeRes()    {return mRes;}

    /** 获取lib */
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
    void scanDir(const string &paths,vector<string> &files); /* 扫描共享库路径*/
    bool checkFile(const string &name); /*检查共享库 */
    int libRegister(const string &name); /*注册共享库*/
    void libUnRegister(const string &name); /*注销共享库 */
    bool chkAllowMod(const string &name); /*检查是否允许文件附加 */

    vector<SHD> mSchHD;
    ResRW mRes;
};


#endif
