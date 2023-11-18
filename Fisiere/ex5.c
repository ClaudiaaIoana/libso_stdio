#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define BUFFER_SIZE 1024



int main(int args, char* argv[])
{
    if(args<2)
    {
        printf("Not enough parameters\n");
        exit(-1);
    }

    char    buffer[BUFFER_SIZE];
    int     fd = open(argv[1], O_RDWR);

    int     characters=read(fd,buffer,BUFFER_SIZE);
    if(characters<0)
    {
        close(fd);
        return -1;
    }

    int     flags=fcntl(fd,F_GETFL);
    flags|= O_NOATIME;
    fcntl(fd,F_SETFL,flags);

    if(write(fd,buffer,characters)!=characters)
    {
        close(fd);
        return -2;
    }

    close(fd);
    return 0;
}