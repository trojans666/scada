#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>
#include <limits.h>

#include "stropt.h"
#include "excep.h"
#include "log.h"
#include "ctrlnode.h"

using namespace SCADA;

CtrlNode::CtrlNode(CtrlNode *iprev)
    :chGrp(NULL)
    ,mUse(0)
    ,mOi(USHRT_MAX)
{
    prev.node = iprev;
    prev.grp = -1;
}

CtrlNode::~CtrlNode()
{
    nodeDelAll();
    if(chGrp)
        delete chGrp;
}

CtrlNode &CtrlNode::operator =(CtrlNode &node)
{
    return *this;
}

void CtrlNode::nodeDelAll()
{
    TMap::iterator p;
    for(unsigned i_g = 0;chGrp && i_g < chGrp->size();i_g++)
    {
        while((p = (*chGrp)[i_g].elem.begin()) != (*chGrp)[i_g].elem.end())
        {
            delete p->second; /** 先释放 map中的value(TCntrNode)指针 */
            (*chGrp)[i_g].elem.erase(p); /** 从map中删除该节点 */
        }
    }
}

string CtrlNode::nodePath(char sep,bool from_root)
{
    if( sep )
    {
        if( prev.node )
        {
            if( from_root && !prev.node->prev.node )
                return ((prev.grp<0)?"":(*(prev.node->chGrp))[prev.grp].id)+nodeName();
            else
                return prev.node->nodePath(sep,from_root)+string(1,sep)+
                        ((prev.grp<0)?"":(*(prev.node->chGrp))[prev.grp].id)+nodeName();
        }
        else return nodeName();
    }
    else
    {
        if( prev.node )
            return prev.node->nodePath(sep,from_root)+
                    ((prev.grp<0)?"":(*(prev.node->chGrp))[prev.grp].id)+nodeName()+"/";
        else return from_root?"/":("/"+nodeName()+"/");
    }
}
/* 节点列表*/
void CtrlNode::nodeList(vector<string> &list,const string &gid)
{
    vector<string> tls;
    list.clear();
    for( unsigned int i_gr = 0; chGrp && i_gr < chGrp->size(); i_gr++ )
    {
        if( gid.empty() || gid == (*chGrp)[i_gr].id )
        {
            /** 拷贝子节点的子节点*/
            chldList(i_gr, tls);
            for( unsigned int i_l = 0; i_l < tls.size(); i_l++ )
                list.push_back((*chGrp)[i_gr].id+tls[i_l]);

            if( !gid.empty() )	break;
        }
    }
}
/* 获取节点*/
AutoHD<CtrlNode> CtrlNode::nodeAt(const string &path,int lev,char sep,int off)
{
    string s_br = sep ? StrOpt::strDecode(StrOpt::strSepParse(path,lev,sep,&off),StrOpt::PathEl) :
            StrOpt::pathLev(path,lev,true,&off);
    if( s_br.empty() )
    {
        return this;
    }
    ResAlloc res(mChildRes,false);
    for( unsigned int i_g = 0; chGrp && i_g < chGrp->size(); i_g++ )
    {
        if( s_br.substr(0,(*chGrp)[i_g].id.size()) == (*chGrp)[i_g].id )
            return chldAt(i_g,s_br.substr((*chGrp)[i_g].id.size())).at().nodeAt(path,0,sep,off);
    }
     //> Go to default group
    if( chGrp )
        return chldAt(0,s_br).at().nodeAt(path,0,sep,off);

    throw TError(nodePath().c_str(),"Node '%s' no present!",s_br.c_str());
}
/* 删除节点 */
void CtrlNode::nodeDel(const string &path,char sep,bool shDel)
{
    AutoHD<CtrlNode> del_n = nodeAt( path, 0, sep );
    int n_grp = del_n.at().prev.grp;

    string n_id  = del_n.at().nodeName();

    del_n = AutoHD<CtrlNode>(del_n.at().prev.node);

    del_n.at().chldDel( n_grp, n_id, -1,shDel );
}

CtrlNode * CtrlNode::nodePrev(bool noex)
{
    if(prev.node)
        return prev.node;
    if(noex)
        return NULL;
    throw TError(nodePath().c_str(),"Node is the root or is not connected!");
}


void CtrlNode::load(bool force) /*如果节点被修改 加载该节点 */
{
    load_(); /* 加载 子类的 */
}
void CtrlNode::save() /*如果节点被修改 保存该节点*/
{

}

/**引用计数 */
void CtrlNode::refConnect()
{
    ResAlloc res(mUserRes,true);
    mUse++;
}

void CtrlNode::refDisConnect()
{
    ResAlloc res(mUserRes,true);
    mUse--;
}


/** 子节点操作  user = 用户*/
AutoHD<CtrlNode> CtrlNode::chldAt(char igr,const string &name,const string &user)
{
    ResAlloc res( mChildRes, false );
    if( !chGrp || (unsigned char)igr >= chGrp->size() )
        throw TError(nodePath().c_str(),"Group of childs %d error!",igr);

    TMap::iterator p = (*chGrp)[igr].elem.find(name);
    if( p == (*chGrp)[igr].elem.end())
    {
        throw TError(nodePath().c_str(),"Element <%s> is not present or disabled!", name.c_str());
    }

    return AutoHD<CtrlNode>(p->second,user);
}

/** igr 组 id 从某个组里找子节点*/
void CtrlNode::chldList(char igr,vector<string> &list)
{
    ResAlloc res(mChildRes,false);
    if( !chGrp || (unsigned char)igr >= chGrp->size() )
        throw TError(nodePath().c_str(),"Group of childs %d error!",igr);

    list.clear();
    list.reserve((*chGrp)[igr].elem.size());

    /** 这里是不排序的 */
    if( !(*chGrp)[igr].ordered )
    {
        for( TMap::iterator p = (*chGrp)[igr].elem.begin(); p != (*chGrp)[igr].elem.end(); ++p )
           // if( p->second->nodeMode() != Disable )
                list.push_back(p->first);
    }
    else
    {
        /** 需要排序 */
        for( TMap::iterator p = (*chGrp)[igr].elem.begin(); p != (*chGrp)[igr].elem.end(); ++p )
            {
                while( p->second->mOi >= list.size() )
                    list.push_back("");
                list[p->second->mOi] = p->first;
            }
    }
}
/** 判断igr组里的name节点是否存在 */
bool CtrlNode::chldPresent(char igr,const string &name)
{
    ResAlloc res(mChildRes,false);
    if( !chGrp || (unsigned char)igr >= chGrp->size() )
        throw TError(nodePath().c_str(),"Group of childs %d error!",igr);

    TMap::iterator p = (*chGrp)[igr].elem.find(name);
    if( p != (*chGrp)[igr].elem.end()) // && p->second->nodeMode() == Enable )
        return true;

    return false;
}

void CtrlNode::chldAdd(char igr, CtrlNode *node, int pos)
{
    ResAlloc res(mChildRes,false);
    if( !chGrp || (unsigned char)igr >= chGrp->size() )
        throw TError(nodePath().c_str(),"Group of childs %d error!",igr);

    TMap::iterator p;
    mess_info("chldAdd","nodeName = %s",node->nodeName().c_str());
    if( StrOpt::strNoSpace(node->nodeName()).empty() )
    {
        delete node;
        throw TError(nodePath().c_str(),"Add child id is empty!");
    }

    if( (p = (*chGrp)[igr].elem.find(node->nodeName())) != (*chGrp)[igr].elem.end() )
    {
        delete node; /** 说明节点已经存在,将传入的节点释放,防止内存泄漏*/
        return;
    }

    res.request(true);
    node->prev.node = this;
    node->prev.grp = igr;
    if( (*chGrp)[igr].ordered ) /** 排序*/
    {
        pos = (pos<0||(unsigned int)pos>(*chGrp)[igr].elem.size()) ? (*chGrp)[igr].elem.size() : pos;
        node->mOi = pos;
        for( p = (*chGrp)[igr].elem.begin(); p != (*chGrp)[igr].elem.end(); p++ )
            if( p->second->mOi >= pos ) p->second->mOi++;
    }
    (*chGrp)[igr].elem.insert(std::pair<string,CtrlNode*>(node->nodeName(),node));
    res.release();
}

void CtrlNode::chldDel(char igr,const string &name,long tm,bool shDel)
{
    if( tm < 0 )	tm = 2;
    ResAlloc res(mChildRes,false);
    if( !chGrp || (unsigned char)igr >= chGrp->size() )
        throw TError(nodePath().c_str(),"Group of childs %d error!",igr);

    TMap::iterator p = (*chGrp)[igr].elem.find(name);
    if( p == (*chGrp)[igr].elem.end() )
        throw TError(nodePath().c_str(),"Child <%s> is not present!", name.c_str());

    if( !shDel )
    {
        res.request( true );
        p = (*chGrp)[igr].elem.find(name);
        if( p == (*chGrp)[igr].elem.end() ) return;
        if( (*chGrp)[igr].ordered )
        {
            int pos = p->second->mOi;
            for( TMap::iterator p1 = (*chGrp)[igr].elem.begin(); p1 != (*chGrp)[igr].elem.end(); ++p1 )
                if( p1->second->mOi > pos ) p1->second->mOi--;
        }
        delete p->second;
        (*chGrp)[igr].elem.erase(p);
    }
}

/** 容器 */
char CtrlNode::grpSize()
{
    return chGrp ? chGrp->size() : 0;
}

char CtrlNode::grpId(const string &sid)
{
    /** 返回group id */
    for( unsigned int g_id = 0; chGrp && g_id < chGrp->size(); g_id++ )
    if( (*chGrp)[g_id].id == sid )	return g_id;

    return -1;
}

CtrlNode::GrpEl &CtrlNode::grpAt(char iid)
{
    /** 根据iid 返回该 group */
    if( iid < 0 || iid >= grpSize( ) )
        throw TError(nodePath().c_str(),"Branche group '%d' error.",iid);

    return (*chGrp)[iid];
}
unsigned CtrlNode::grpAdd(const string &iid,bool iordered)
{
    /** 创建一个 group */
    if( !chGrp ) chGrp = new vector<GrpEl>;

    unsigned int g_id;
    /** 查看iid是否在group中*/
    for( g_id = 0; g_id < chGrp->size(); g_id++ )
        if( (*chGrp)[g_id].id == iid ) break;
    /** 说明没有iid */
    if( g_id == chGrp->size() )	chGrp->push_back( GrpEl() );
    /** 这里无论是创建还是已存在 都以找到该节点 */
    (*chGrp)[g_id].id = iid;
    (*chGrp)[g_id].ordered = iordered;

    return g_id;
}


