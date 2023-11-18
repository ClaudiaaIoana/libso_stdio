#include"so_stdio.h"
#include<stdio.h>

int main()
{
    SO_FILE* f;
    f=so_fopen("in.txt","r+");
    
    char buffer[10];
    so_fread(buffer,sizeof(char),10,f);
    printf("%s",buffer);

    so_fputc('?',f);

    so_fread(buffer,sizeof(char),10,f);
    printf("%s\n",buffer);


    //char c=so_fgetc(f);
    //printf("%c\n",c);

    so_fclose(f);

}