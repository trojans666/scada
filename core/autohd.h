
#ifndef __AUTOHD_H
#define __AUTOHD_H

#include <stdio.h>
#include <string>

#include "excep.h"

/**
 *@brief
 */
namespace SCADA
{

template<typename ORes>
class AutoHD
{
public:
    /** 初始化 但不连接到对象 */
    AutoHD():mNode(NULL) {}
    /** 初始化且连接到对象 该对象必须实现 AHDConnect */
    AutoHD(ORes *node,const string &who=""):mNode(node)
    {
        if(mNode)
            mNode->refConnect();
    }
    /** 拷贝构造 */
    AutoHD(const AutoHD &hd):mNode(NULL)
    {
        operator=(hd);
    }
    /** 利用dynamic_cast 将hd_s转换为 ORes1 用在类与类转换中 */
    template<typename ORes1> AutoHD(const AutoHD<ORes1> &hd_s,bool nosafe=false):mNode(NULL)
    {
        if(hd_s.freeStat())
            return ;
        if(nosafe)
            mNode = (ORes *)&hd_s.at();
        else
        {
            mNode = dynamic_cast<ORes *>(&hd_s.at());
            if(!mNode)
                throw TError("AutoHD","Type casting error!");
        }
        mNode->refConnect();
    }

    ~AutoHD() {free_hd();}

    ORes &at() const
    {
        if(mNode)
            return *mNode;
        throw TError("AutoHD","No init!");
    }
    void operator=(const AutoHD &hd)
    {
        free_hd();
        mNode = hd.mNode;
        if(mNode)
            mNode->refConnect();
    }
    /** 释放资源 */
    void free_hd()
    {
        if(mNode)
            mNode->refDisConnect();
        mNode = NULL;
    }

    bool freeStat() const
    {
        return (mNode == NULL) ? true : false;
    }
private:
    ORes *mNode;
};

}

#endif // __AUTOHD_H
