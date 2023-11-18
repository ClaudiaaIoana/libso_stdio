#include<stdio.h>
#include <fcntl.h>
#include <unistd.h>

extern int my_open(char* filename, int flags, int mode);
extern int my_read(unsigned int fd, char* buf,size_t size);
extern int my_write(unsigned int fd, char*buffer, size_t count);
extern int my_close(unsigned int fd);

#define BUFFER_SIZE 16

int main()
{
    char buffer[32];

    int _fd=my_open("in.txt", O_RDONLY| O_CREAT,0644);
    my_read(_fd,buffer,BUFFER_SIZE);
    my_write(1,buffer,BUFFER_SIZE);

    my_close(_fd);

}