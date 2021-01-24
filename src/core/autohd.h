
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
    /** ��ʼ�� �������ӵ����� */
    AutoHD():mNode(NULL) {}
    /** ��ʼ�������ӵ����� �ö������ʵ�� AHDConnect */
    AutoHD(ORes *node,const string &who=""):mNode(node)
    {
        if(mNode)
            mNode->refConnect();
    }
    /** �������� */
    AutoHD(const AutoHD &hd):mNode(NULL)
    {
        operator=(hd);
    }
    /** ����dynamic_cast ��hd_sת��Ϊ ORes1 ����������ת���� */
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
    /** �ͷ���Դ */
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
