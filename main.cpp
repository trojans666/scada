#include "excep.h"
#include "sys.h"


using std::string;

int main(int argc,char *argv[],char **envp)
{
    int rez = 0;

    try
    {
        sys = new SYS(argc,argv,envp);
        sys->load();
        /* �������� substart,ÿ��substart������ڲ����е�modstart */
        rez = sys->start();

        delete sys;
    }
    catch(TError err)
    {

    }

    return rez;
}
