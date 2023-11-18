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
        printf("%d ",i);
        exit(1);
    }	
	if (pid > 0)
    {
        return;
    }
}

int main(int args, char* argv[])
{
    if(args<2)
    {
        printf("Not enough arguments!\n");
        exit(-1);
    }
    int     num_process;
    num_process=atoi(argv[1]);
    for(int i=1;i<=num_process;i++)
    {
        create_process(i);
    }

    int signal;
    wait(&signal);
}