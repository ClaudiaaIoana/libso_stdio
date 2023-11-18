#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define BUFFER_SIZE 10

void write_random(int fd)
{
    srand(time(NULL));
    char characters[BUFFER_SIZE+1];
    for(int i=0;i<10;i++)
    {
        int letters=('Z'-'A')*2;
        int chr = rand() % letters; 

        if ( chr < letters/2) {
            characters[i]= 'A' + chr;
        } else {
            characters[i]= 'a' + (chr % 26); 
        }
    }
    if(write(fd,characters,BUFFER_SIZE)!=10)
    {
        perror("Error writing random characters!\n");
    }
}


int main(int args, char* argv[])
{
    if(args<2)
    {
        printf("Not enough parameters\n");
        exit(-1);
    }

    int     fd = open(argv[1], O_WRONLY | O_CREAT, 0644);

    ftruncate(fd,10);

    write_random(fd);

    close(fd);
    return 0;
}