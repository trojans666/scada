#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "xml.h"

using std::string;

int main(int argc,char *argv[])
{
    int hd = open("oscada.xml",O_RDONLY);
    if(hd < 0)
        printf("error");
    else
    {
        string s_buf;
        int cf_sz = lseek(hd,0,SEEK_END);
        if(cf_sz > 0)
        {
            lseek(hd,0,SEEK_SET);
            char *buf = (char *)malloc(cf_sz+1);
            read(hd,buf,cf_sz);
            buf[cf_sz] = 0;
            s_buf = buf;
            free(buf);
        }
        close(hd);

        XMLNode root;
        root.load(s_buf);

        if(root.name() == "OpenSCADA")
        {
            std::cout << "fjdslkfj" << std::endl;
            for(int i_st = root.childSize() - 1; i_st >= 0; i_st--)
            {
                std::cout << root.childGet(i_st)->name() << std::endl;
                if(root.childGet(i_st)->name() == "station")
                {
                    XMLNode *stat_n = root.childGet(i_st);
                    std::cout << stat_n->attr("id") << std::endl;
                    if(stat_n->attr("id") == "WorkStation")
                    {
                        std::cout << "1111" << std::endl;
                    }
                }
            }
        }
    }
}
