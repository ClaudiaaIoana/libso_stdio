#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 16

int main()
{
    char    buffer[BUFFER_SIZE+1];

    int     fd=open("in.txt", O_RDONLY | O_CREAT, 0644);
    int     num_characters;
    
    num_characters=read(fd,buffer,BUFFER_SIZE);

    if(num_characters<0)
    {
        close(fd);
        return -1;
    }

    if(write(1,buffer,num_characters)!=num_characters)
    {
        close(fd);
        return -2;
    }

    close(fd);
    return 0;
}