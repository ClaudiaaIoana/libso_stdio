#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 20

int main(int args, char* arg[])
{
    if(args<2)
    {
        printf("Not enough parameters\n");
        exit(-1);
    }

    char    buffer[BUFFER_SIZE+1];
    int     fd=open(arg[1],O_NOFOLLOW | O_RDONLY );

    if(fd<0)
    {
        perror("Error opening the file");
        exit(-1);
    }

    lseek(fd,-20,SEEK_END);
    int     characters=read(fd,buffer,BUFFER_SIZE);

    if(characters<0)
    {
        close(fd);
        return -1;
    }

    if(write(1,buffer,characters)!=characters)
    {
        close(fd);
        return -2;
    }

    close(fd);
    return 0;
}