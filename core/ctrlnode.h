/**
控制节点类 所有节点的父类
*/


#ifndef __CTRLNODE_H
#define __CTRLNODE_H

#include <string>
#include <vector>
#include <map>

#include "resalloc.h"
#include "autohd.h"

using std::string;
using std::vector;
using std::map;

class CtrlNode;

typedef map<string,CtrlNode *> TMap;


class CtrlNode
{
public:
    CtrlNode(CtrlNode *iprev = NULL);
    virtual ~CtrlNode();

    virtual CtrlNode &operator=(CtrlNode &node);


    virtual ResRW &UserRes() {return mUserRes;}
    virtual const string &nodeName() = 0; /*节点名称*/
    string nodePath(char sep = 0,bool from_root = false); /*节点路径*/
    /*节点列表*/
    void nodeList(vector<string> &list,const string &gid = "");
    /* 获取节点*/
    AutoHD<CtrlNode> nodeAt(const string &path,int lev = 0,char sep = 0,int off = 0);
    /* 删除节点 */
    void nodeDel(const string &path, char sep = 0, bool shDel = 0);

    CtrlNode *nodePrev(bool noex = false);


    /* 节点的引用计数 */
    unsigned nodeUse()  {return mUse;}
    /* 节点的排序 */
    unsigned nodePos() {return mOi;}

    void load(bool force = false); /* 加载 */
    void save(); /* 保存 */

    /* 引用计数 */
    virtual void refConnect();
    virtual void refDisConnect();

protected:
    struct GrpEl
    {
        string id;
        bool ordered; /*排序*/
        TMap elem;
    };

    void nodeDelAll(); /* 删除所有节点 */
    void setNodePrev(CtrlNode *node) {prev.node = node;}

    /* 获取子节点 */
    virtual AutoHD<CtrlNode> chldAt(char igr,const string &name,const string &user = "");
    void chldList(char igr,vector<string> &list);

    bool chldPresent(char igr,const string &name); /* 检查节点是否存在 */
    void chldAdd(char igr,CtrlNode *node, int pos = -1);
    void chldDel(char igr, const string &name, long tm = -1, bool shDel = false);

    /** 容器 */
    char grpSize();
    char grpId(const string &sid);
    GrpEl &grpAt(char id);
    unsigned grpAdd(const string &id,bool ordered = false);

    /* 在load中被调用 */
    virtual void load_() {}
    virtual void save_() {}

private:
    struct
    {
        CtrlNode *node;
        char grp;
    }prev; /* 节点链表 */
    ResRW mChildRes; /* 子节点锁 */
    ResRW mUserRes; /* 用户操作锁 */

    /* 一个group可能有多个节点 每个节点又有多个元素 */
    vector<GrpEl> *chGrp; /** child groups */

    unsigned short int mUse; /* 引用计数 */
    unsigned short int mOi; /*order index 排序 */
};


#endif
