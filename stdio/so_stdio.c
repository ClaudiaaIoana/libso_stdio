#include"so_stdio.h"
#include<fcntl.h>
#include<string.h>
#include <unistd.h>

//TODO:
#include<stdio.h>

#define     DEFAULT_BUFF_SIZE   4096

#define     LAST_READ           0
#define     LAST_WRITE          1

#define     OPEN_RD             0
#define     OPEN_RD_U           1
#define     OPEN_WR             2
#define     OPEN_WR_U           3
#define     OPEN_APP            4
#define     OPEN_APP_U          5


struct _so_file
{
    int     file_descriptor;
    char*   buffer;
    int     last_operation;
    int     buff_cursor;
    int     file_cursor;
    int     open_mode;
    int     buff_len;
};

int write_buffer_into_file(SO_FILE* stream)
{
    //write the buffer content in
    int     num_chr;
    num_chr=write(stream->file_descriptor,stream->buffer,stream->buff_len);

    if(num_chr!=stream->buff_len)
    {
        return SO_EOF;
    }
    free(stream->buffer);
    stream->buffer=NULL;
    stream->buff_cursor=0;
    stream->buff_len=0;

    return 0;
}

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
    SO_FILE*    f=NULL;
    int         fd;
    mode_t      mode_open=0;
    int         open_mode;

    if(strcmp(mode,"r")==0)
    {
        mode_open|=O_RDONLY;
        open_mode=OPEN_RD;
    }
    else if (strcmp(mode,"r+")==0)
    {
        mode_open|=O_RDWR;
        open_mode=OPEN_RD_U;
    }
    else if(strcmp(mode,"w")==0)
    {
        mode_open|=O_WRONLY|O_CREAT | O_TRUNC;
        open_mode=OPEN_WR;
    }
    else if(strcmp(mode,"w+")==0)
    {
        mode_open|=O_RDWR|O_CREAT|O_TRUNC;
        open_mode=OPEN_WR_U;
    }
    else if(strcmp(mode,"a")==0)
    {
        mode_open|=O_WRONLY|O_APPEND|O_CREAT;
        open_mode=OPEN_APP;
    }
    else if (strcmp(mode,"a+")==0)
    {
        mode_open|=O_RDWR|O_APPEND|O_CREAT;
        open_mode=OPEN_APP_U;
    }

    fd=open(pathname,mode_open, 0644);
    if(fd<0)
    {
        return NULL;
    }

    f=(SO_FILE*)malloc(sizeof(SO_FILE));
    f->file_descriptor=fd;
    f->buff_cursor=0;
    f->file_cursor=0;
    f->last_operation=-1;
    f->open_mode=open_mode;
    f->buffer=NULL;

    return f;
}

int so_fclose(SO_FILE *stream)
{
    int state=0;

    if(stream->buffer && stream->last_operation==LAST_WRITE)
    {
        if(write_buffer_into_file(stream)==SO_EOF)
            state=SO_EOF;
    }

    if(close(stream->file_descriptor)!=0)
    {
        state=SO_EOF;
    }

    if(!stream->buffer)
    {
        free(stream->buffer);
        stream->buffer=NULL;
    }
    free(stream);

    return state;
}

int so_fgetc(SO_FILE *stream)
{
    //doesn't have permision to read
    if(stream->open_mode>=OPEN_WR && stream->open_mode%2==0)
    {
        return SO_EOF;
    }

    //the file was oppened with update (+) 
    //the last operation was a write
    if(stream->open_mode%2==1 && stream->last_operation==LAST_WRITE)
    {
        write_buffer_into_file(stream);
    }

    //when the buffer was invalidated
    if(!stream->buffer)
    {
        stream->buffer=(char*)calloc(DEFAULT_BUFF_SIZE,sizeof(char));
        if(stream->buffer==NULL)
        {
            return SO_EOF;
        }
    }

    //when the cursor if the begining of the buffer
    if(stream->buff_cursor==0)
    {
        int     num_char;
        num_char=read(stream->file_descriptor, stream->buffer, DEFAULT_BUFF_SIZE );
        if(num_char<=0)
        {
            return SO_EOF;
        }
        stream->buff_len=num_char;
    }

    unsigned char   c;
    c=stream->buffer[stream->buff_cursor];

    stream->buff_cursor++;
    stream->last_operation=LAST_READ;

    if(stream->buff_cursor==stream->buff_len)
    {
        memset(stream->buffer,DEFAULT_BUFF_SIZE,sizeof(char));
        stream->buff_cursor=0;
        stream->buff_len=0;
    }

    return c;
}

int so_fputc(int c, SO_FILE *stream)
{
    //doesn't have permision to write
    if(stream->open_mode==OPEN_RD)
    {
        return SO_EOF;
    }

    //invalid buffer
    if(!stream->buffer)
    {
        stream->buffer=(char*)calloc(DEFAULT_BUFF_SIZE,sizeof(char));
        if(stream->buffer==NULL)
        {
            return SO_EOF;
        }
    }

    //the file was oppened with update (+) 
    //the last operation was a read
    if(stream->last_operation==LAST_READ && stream->open_mode%2==1)
    {
        //move the file cursor to the effective place
        //if the open mode was append then don't move the cursor, it will authomaticly be moved at EOF when writing
        if(stream->open_mode!=OPEN_APP_U)
        {
            int     go_back=stream->buff_cursor-stream->buff_len;
            lseek(stream->file_descriptor,go_back,SEEK_CUR);
        }

        //reset buff status
        memset(stream->buffer,DEFAULT_BUFF_SIZE,sizeof(char));
        stream->buff_cursor=0;
        stream->buff_len=0;
    }

    //write character into buffer
    stream->buffer[stream->buff_cursor]=c;
    stream->buff_cursor++;
    stream->buff_len++;
    stream->last_operation=LAST_WRITE;

    //full buffer
    if(stream->buff_cursor==DEFAULT_BUFF_SIZE)
    {
        write_buffer_into_file(stream);
    }

    return c;

}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    //doesn't have permision to read
    if(stream->open_mode>=OPEN_WR && stream->open_mode%2==0)
    {
        return 0;
    }

    size_t      bytes_size=size/sizeof(char);         
    size_t      elements_read=0;
    char        character;
    char        element[bytes_size+1];

    while(elements_read < nmemb)
    {
        for(int i=0;i<bytes_size;i++)
        {
            character=so_fgetc(stream);
            if(character<0)
            {
                break;
            }
            element[i]=character;
        }
        element[bytes_size]='\0';
        if(character<0)
        {
            break;
        }
        strcpy(ptr+elements_read*bytes_size,element);
        elements_read++;
    }
    return elements_read;
}