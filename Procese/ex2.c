#include<stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include<sys/wait.h>

void create_process(int i)
{
    int     pid;
    setvbuf(stdout, NULL, _IONBF, 0);
	pid = fork();

    if(pid<0)
    {
        printf("Creating child process %d failed!\n",i);
        exit(-1);
    }
	if (pid == 0)
	{
        
    }	
	if (pid > 0)
    {
        return;
    }
}

int main(int args, char* argv[])
{
    if(args<4)
    {
        printf("Not enough arguments!\n");
        exit(-1);
    }
    
    int     fd=open(argv[2],O_RDONLY);
    int     searched=atoi(argv[1]);
    int     num_proc=atoi(argv[3]);
    int     num_numbers;

    

}