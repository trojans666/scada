#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>

#ifdef __linux__
#include <dlfcn.h>
#endif

#include "sys.h"
#include "stropt.h"
#include "modschedul.h"

ModSchedul::ModSchedul()
    :SubSys(MODSCHEDUL_ID,MODSCHEDUL_NAME,false)
{

}

int ModSchedul::loadLibS()
{
    return libLoad(sys->modDir(),false);
}


void ModSchedul::load_()
{

}

void ModSchedul::save_()
{

}

void ModSchedul::scanDir(const std::string &Paths, vector<std::string> &files)
{
    string NameMod, Path;

    files.clear();

    //> Check and append present files
    int ido, id=-1;
    do
    {
        ido=id+1; id = Paths.find(",",ido);

        dirent *scan_dirent;
        Path=Paths.substr(ido,id-ido);
        if(Path.size() <= 0) continue;

        DIR *IdDir = opendir(Path.c_str());
        if(IdDir == NULL) continue;

        while((scan_dirent = readdir(IdDir)) != NULL)
        {
            if(string("..") == scan_dirent->d_name || string(".") == scan_dirent->d_name) continue;
            NameMod = Path+"/"+scan_dirent->d_name;
            if(checkFile(NameMod)) files.push_back(NameMod);
        }
        closedir(IdDir);

    } while(id != (int)string::npos);
}

bool ModSchedul::checkFile(const std::string &iname)
{
    struct stat file_stat;
    string NameMod;

    stat(iname.c_str(),&file_stat);

    if(iname.size() <= 3 || iname.compare(iname.size()-3,3,".so") != 0 || (file_stat.st_mode&S_IFMT) != S_IFREG) return false;

    if(access(iname.c_str(),F_OK|R_OK) != 0)  return false;

    NameMod = iname;

    /*void *h_lib = dlopen(iname.c_str(),RTLD_LAZY|RTLD_LOCAL);
    if( h_lib == NULL )
    {
    //mess_warning(nodePath().c_str(),_("Module <%s> error: %s !"),iname.c_str(),dlerror());
    return false;
    }
    else dlclose(h_lib);*/

    ResAlloc res(nodeRes(),false);
    for(unsigned int i_sh=0; i_sh < mSchHD.size(); i_sh++)
        if(mSchHD[i_sh].name == iname)
        {
            if(file_stat.st_mtime > mSchHD[i_sh].tm)
                return true;
            else
                return false;
        }


    return true;
}

int ModSchedul::libRegister(const std::string &name)
{
    struct stat file_stat;

    ResAlloc res(nodeRes(),true);
    stat(name.c_str(),&file_stat);
    unsigned i_sh;

    for(i_sh = 0; i_sh < mSchHD.size(); i_sh++)
        if(mSchHD[i_sh].name == name)
            break;

    if(i_sh >= mSchHD.size())
        mSchHD.push_back(SHD(NULL,file_stat.st_mtime,name));
    else
        mSchHD[i_sh].tm = file_stat.st_mtime;

    return i_sh;
}

void ModSchedul::libUnRegister(const std::string &iname)
{
    ResAlloc res(nodeRes(),true);
    for(unsigned i_sh = 0; i_sh < mSchHD.size(); i_sh++)
        if(mSchHD[i_sh].name == iname)
        {
            if(mSchHD[i_sh].hd)
                libDet(iname);
            mSchHD.erase(mSchHD.begin()+i_sh);
            return;
        }
    throw TError(nodePath().c_str(),"SO <%s> is not present!",iname.c_str());
}

void ModSchedul::libAtt(const std::string &iname, bool full)
{
    char *dlErr = NULL;
    ResAlloc res(nodeRes(),true);
    for(unsigned int i_sh = 0; i_sh < mSchHD.size(); i_sh++)
        if(mSchHD[i_sh].name == iname)
        {
            if(mSchHD[i_sh].hd)
                throw TError(nodePath().c_str(),"SO <%s> is already attached!",iname.c_str());

            void *h_lib = dlopen(iname.c_str(),RTLD_LAZY|RTLD_LOCAL);
            if(!h_lib)
            {
                mSchHD[i_sh].err = dlerror();
                throw TError(nodePath().c_str(),"SO <%s> error: %s !",iname.c_str(),mSchHD[i_sh].err.c_str());
            }

            //> Connect to module function
            Module::SAt (*module)( int );
            module = (Module::SAt (*)(int)) dlsym(h_lib,"module");
            if((dlErr=dlerror()) != NULL)
            {
                mSchHD[i_sh].err = dlErr;
                dlclose(h_lib);
                throw TError(nodePath().c_str(),"SO <%s> error: %s !",iname.c_str(),mSchHD[i_sh].err.c_str());
            }

            //> Connect to attach function
            Module *(*attach)( const Module::SAt &, const string & );
            attach = (Module * (*)(const Module::SAt &, const string &)) dlsym(h_lib,"attach");
            if((dlErr=dlerror()) != NULL)
            {
                mSchHD[i_sh].err = dlErr;
                dlclose(h_lib);
                throw TError(nodePath().c_str(),"SO <%s> error: %s !",iname.c_str(),mSchHD[i_sh].err.c_str());
            }

            //> Get allow modules from library and start it
            int n_mod=0, add_mod=0;
            Module::SAt AtMod;
            while((AtMod = (module)(n_mod++)).id.size())
            {
                vector<string> list;
                owner().list(list);
                for(unsigned i_sub = 0; i_sub < list.size(); i_sub++)
                {
                    if(owner().at(list[i_sub]).at().subModule() && AtMod.type == owner().at(list[i_sub]).at().subId())
                    {
                        //>> Check type module version
                        if( AtMod.t_ver != owner().at(list[i_sub]).at().subVer() )
                        {
                           // mess_warning(nodePath().c_str(),"%s for type <%s> doesn't support module version: %d!",
                           //              AtMod.id.c_str(),AtMod.type.c_str(),AtMod.t_ver);
                            break;
                        }
                        //>> Check module present
                        if( owner().at(list[i_sub]).at().modPresent(AtMod.id) )
                           ;// mess_warning(nodePath().c_str(),"Module <%s> is already present!",AtMod.id.c_str());
                        else
                        {
                            //>> Attach new module
                            Module *LdMod = (attach)( AtMod, iname );
                            if(LdMod == NULL)
                            {
                               // mess_warning(nodePath().c_str(),"Attach module <%s> error!",AtMod.id.c_str());
                                break;
                            }
                            //>> Add atached module
                            owner().at(list[i_sub]).at().modAdd(LdMod);
                            mSchHD[i_sh].use.push_back(list[i_sub]+"."+LdMod->modId());
                            if(full)
                            {
                                owner().at(list[i_sub]).at().load(true);
                                owner().at(list[i_sub]).at().subStart();
                            }
                            add_mod++;
                            break;
                        }
                    }
                }
            }
            if(add_mod == 0)
                dlclose(h_lib);
            else
                mSchHD[i_sh].hd = h_lib;
            return;
        }
    throw TError(nodePath().c_str(),"SO <%s> is not present!",iname.c_str());
}

void ModSchedul::libDet(const std::string &iname)
{
    ResAlloc res(nodeRes(),true);
    for(unsigned int i_sh = 0; i_sh < mSchHD.size(); i_sh++)
        if(mSchHD[i_sh].name == iname)
        {
            if(!mSchHD[i_sh].hd)
                return;
            try
            {
                //> Stop all modules
                for(unsigned int i_m = 0; i_m < mSchHD[i_sh].use.size(); i_m++)
                    owner().at(StrOpt::strSepParse(mSchHD[i_sh].use[i_m],0,'.')).at().
                            modAt(StrOpt::strSepParse(mSchHD[i_sh].use[i_m],1,'.')).at().modStop();
                //> Delete all modules
                while(mSchHD[i_sh].use.size())
                {
                    owner().at(StrOpt::strSepParse(mSchHD[i_sh].use[0],0,'.')).at().
                            modDel(StrOpt::strSepParse(mSchHD[i_sh].use[0],1,'.'));

                    mSchHD[i_sh].use.erase(mSchHD[i_sh].use.begin());
                }
            }catch(TError err)
            {
                //owner().at(SchHD[i_sh]->use[0].mod_sub).at().modAt(SchHD[i_sh]->use[0].n_mod).at().load();
                //> Start all modules
                for(unsigned int i_m = 0; i_m < mSchHD[i_sh].use.size(); i_m++)
                    owner().at(StrOpt::strSepParse(mSchHD[i_sh].use[i_m],0,'.')).at().
                            modAt(StrOpt::strSepParse(mSchHD[i_sh].use[i_m],1,'.')).at().modStart();
                throw;
            }
            dlclose(mSchHD[i_sh].hd);
            mSchHD[i_sh].hd = NULL;
            return;
        }
    throw TError(nodePath().c_str(),"SO <%s> is not present!",iname.c_str());
}

void ModSchedul::libList(vector<std::string> &list)
{
    ResAlloc res(nodeRes(),false);
    list.clear();
    for(unsigned i_sh = 0; i_sh < mSchHD.size(); i_sh++)
        list.push_back(mSchHD[i_sh].name);
}

ModSchedul::SHD ModSchedul::lib(const std::string &iname)
{
    ResAlloc res(nodeRes(),false);
    for(unsigned i_sh = 0; i_sh < mSchHD.size(); i_sh++)
        if(mSchHD[i_sh].name == iname)
            return mSchHD[i_sh];
    throw TError(nodePath().c_str(),"SO <%s> is not present!",iname.c_str());
}

int ModSchedul::libLoad(const std::string &iname, bool full)
{
    int ldCnt = 0;
    vector<string> files, llist;

    //> 获取所有lib库
    scanDir(iname, files);

    //> 库添加处理
    for(unsigned int i_f = 0; i_f < files.size(); i_f++)
    {
        unsigned int i_sh;

        ResAlloc res(nodeRes(),false);
        for(i_sh = 0; i_sh < mSchHD.size(); i_sh++)
            if(mSchHD[i_sh].name == files[i_f])
                break; /*这里说明已经加载了*/

        if(i_sh < mSchHD.size())
        {
            try
            {
                res.release();
                if(1)  /*都允许*/
                    libDet(files[i_f]);
            }
            catch(TError err)
            {
               // mess_warning(err.cat.c_str(),"%s",err.mess.c_str());
               // mess_warning(nodePath().c_str(),"Can't detach library <%s>.",files[i_f].c_str());
                continue;
            }
        }
        res.release();
        libRegister(files[i_f]);
        if(1) /*都允许*/
        {
            try
            {
                libAtt(files[i_f],full); ldCnt++;
            }
            catch(TError err)
            {
               // mess_warning(err.cat.c_str(),"%s",err.mess.c_str());
            }
        }
    }

    return ldCnt;
}
