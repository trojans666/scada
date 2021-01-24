/**
���ƽڵ��� ���нڵ�ĸ���
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

namespace SCADA
{


class CtrlNode;

typedef map<string,CtrlNode *> TMap;


class CtrlNode
{
public:
    CtrlNode(CtrlNode *iprev = NULL);
    virtual ~CtrlNode();

    virtual CtrlNode &operator=(CtrlNode &node);

    enum Flag
    {
        Enable = (1U << 0),
        Disable = (1U << 1),
        Delete = (1U << 2),
        Modify = (1U << 3)
    };
    unsigned char &nodeFlg()   {return mFlg;}
    bool isModify() { return ((mFlg ^ (~Modify)) == 1) ? true : false;}
    void modifClr() {mFlg &= ~(Modify);}

    virtual ResRW &UserRes() {return mUserRes;}
    virtual const string &nodeName() = 0; /*�ڵ�����*/
    string nodePath(char sep = 0,bool from_root = false); /*�ڵ�·��*/
    /*�ڵ��б�*/
    void nodeList(vector<string> &list,const string &gid = "");
    /* ��ȡ�ڵ�*/
    AutoHD<CtrlNode> nodeAt(const string &path,int lev = 0,char sep = 0,int off = 0);
    /* ɾ���ڵ� */
    void nodeDel(const string &path, char sep = 0, bool shDel = 0);

    CtrlNode *nodePrev(bool noex = false);


    /* �ڵ�����ü��� */
    unsigned nodeUse()  {return mUse;}
    /* �ڵ������ */
    unsigned nodePos() {return mOi;}

    void load(bool force = false); /* ���� */
    void save(); /* ���� */

    /* ���ü��� */
    virtual void refConnect();
    virtual void refDisConnect();

protected:
    struct GrpEl
    {
        string id;
        bool ordered; /*����*/
        TMap elem;
    };

    void nodeDelAll(); /* ɾ�����нڵ� */
    void setNodePrev(CtrlNode *node) {prev.node = node;}

    /* ��ȡ�ӽڵ� */
    virtual AutoHD<CtrlNode> chldAt(char igr,const string &name,const string &user = "");
    void chldList(char igr,vector<string> &list);

    bool chldPresent(char igr,const string &name); /* ���ڵ��Ƿ���� */
    void chldAdd(char igr,CtrlNode *node, int pos = -1);
    void chldDel(char igr, const string &name, long tm = -1, bool shDel = false);

    /** ���� */
    char grpSize();
    char grpId(const string &sid);
    GrpEl &grpAt(char id);
    unsigned grpAdd(const string &id,bool ordered = false);

    /* ��load�б����� */
    virtual void load_() {}
    virtual void save_() {}

private:
    struct
    {
        CtrlNode *node;
        char grp;
    }prev; /* �ڵ����� */
    ResRW mChildRes; /* �ӽڵ��� */
    ResRW mUserRes; /* �û������� */

    /* һ��group�����ж���ڵ� ÿ���ڵ����ж��Ԫ�� */
    vector<GrpEl> *chGrp; /** child groups */

    unsigned short int mUse; /* ���ü��� */
    unsigned short int mOi; /*order index ���� */
    unsigned char mFlg; /*�ڵ��־ */
};

}

#endif
