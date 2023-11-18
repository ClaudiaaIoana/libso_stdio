#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int get_perminion(char* perm_string)
{
    int perm=00;
    for(int i =0;i<3;i++)
    {
        perm*=8;
        if(perm_string[0]=='r')
        {
            perm+=4;
        }
        if(perm_string[1]=='w')
        {
            perm+=2;
        }
        if(perm_string[2]=='x')
        {
            perm+=1;
        }

        perm_string+=3;
    }

    return perm;
}

int main(int args, char* argv[])
{
    if(args<3)
    {
        printf("Not enough parameters\n");
        exit(-1);
    }

    int     perm=get_perminion(argv[2]);
    umask(0);

    int     fd = open(argv[1], O_EXCL | O_CREAT, perm);
    
    if(fd<0)
    {
        printf("Existing file!\n");
    }
    else{
        printf("File created!\n");
    }

    close(fd);
    return 0;
}