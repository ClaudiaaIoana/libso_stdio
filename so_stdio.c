#include"so_stdio.h"
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<errno.h>

#define	DEFAULT_BUFF_SIZE	4096

#define	LAST_READ			0
#define	LAST_WRITE		1

#define	OPEN_RD			0
#define	OPEN_RD_U			1
#define	OPEN_WR			2
#define	OPEN_WR_U			3
#define	OPEN_APP			4
#define	OPEN_APP_U		5

#define	READ_END			0
#define	WRITE_END			1


struct _so_file {

	char		*buffer;
	int		file_descriptor;
	int		last_operation;
	int		buff_cursor;
	int		error_occured;
	int		open_mode;
	int		buff_len;
	int		associated_pid;
	int		end_file;
};

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE		*f = NULL;
	int			fd;
	mode_t		mode_open = 0;
	int			open_mode;

	if (strcmp(mode, "r") == 0) {
		mode_open |= O_RDONLY;
		open_mode = OPEN_RD;
	} else if (strcmp(mode, "r+") == 0) {
		mode_open |= O_RDWR;
		open_mode = OPEN_RD_U;
	} else if (strcmp(mode, "w") == 0) {
		mode_open |= O_WRONLY | O_CREAT | O_TRUNC;
		open_mode = OPEN_WR;
	} else if (strcmp(mode, "w+") == 0) {
		mode_open |= O_RDWR | O_CREAT | O_TRUNC;
		open_mode = OPEN_WR_U;
	} else if (strcmp(mode, "a") == 0) {
		mode_open |= O_WRONLY | O_APPEND | O_CREAT;
		open_mode = OPEN_APP;
	} else if (strcmp(mode, "a+") == 0) {
		mode_open |= O_RDWR | O_APPEND | O_CREAT;
		open_mode = OPEN_APP_U;
	} else {
		f = NULL;
		return NULL;
	}

	fd = open(pathname, mode_open, 0644);
	if (fd < 0) {
		f = NULL;
		return NULL;
	}

	f = (SO_FILE *)malloc(sizeof(SO_FILE));
	f->file_descriptor = fd;
	f->buff_cursor = 0;
	f->buff_len = 0;
	f->last_operation = -1;
	f->open_mode = open_mode;
	f->buffer = NULL;
	f->buffer = (char *)calloc(DEFAULT_BUFF_SIZE+1, sizeof(char));
	if (f->buffer == NULL) {
		close(f->file_descriptor);
		free(f);
		f = NULL;
		return NULL;
	}
	f->error_occured = 0;
	f->associated_pid = 0;
	f->end_file = 0;

	return f;
}


int so_fclose(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	int state = 0;

	if (stream->buffer && stream->last_operation == LAST_WRITE) {
		if (so_fflush(stream) == SO_EOF) {
			state = SO_EOF;
			stream->error_occured = 1;
		}
	}

	if (close(stream->file_descriptor) != 0) {
		stream->error_occured = 1;
		state = SO_EOF;
	}

	if (stream->buffer) {
		free(stream->buffer);
		stream->buffer = NULL;
	}
	free(stream);
	stream = NULL;

	return state;
}


int so_fgetc(SO_FILE *stream)
{
	//////////////////////////////////
	//invalid stream
	if (stream == NULL)
		return SO_EOF;
	//the file was oppened with update (+)
	//the last operation was a write
	if (stream->open_mode%2 == 1 && stream->last_operation == LAST_WRITE) {
		//to write de buffer content into the file
		//the cusor will be authomaticly be moved by the write function
		so_fseek(stream, 0, SEEK_CUR);
	}

	//when the cursor if the begining of the buffer
	//no character has been read from the buffer => the buffer is empty
	if (stream->buff_cursor == 0) {
		int		num_char;

		num_char = read(stream->file_descriptor, stream->buffer, DEFAULT_BUFF_SIZE);
		if (num_char < 0) {
			stream->error_occured = 1;
			return SO_EOF;
		}
		if (num_char == 0) {
			stream->end_file = 1;
			return SO_EOF;
		}
		stream->buff_len = num_char;
	}

	unsigned char	c;

	c = stream->buffer[stream->buff_cursor];
	stream->buff_cursor++;
	stream->last_operation = LAST_READ;

	//at the end of the buffer
	if (stream->buff_cursor == stream->buff_len) {
		//reset buff status
		memset(stream->buffer, '\0', DEFAULT_BUFF_SIZE);
		stream->buff_cursor = 0;
		stream->buff_len = 0;
	}

	return c;
}


int so_fputc(int c, SO_FILE *stream)
{
	//invalid stream
	if (stream == NULL)
		return SO_EOF;
	//doesn't have permision to write
	if (stream->open_mode == OPEN_RD)
		return SO_EOF;
	//the file was oppened with update (+)
	//the last operation was a read
	if (stream->last_operation == LAST_READ && stream->open_mode%2 == 1) {
		//move the file cursor to the effective place
		int	go_back = stream->buff_cursor-stream->buff_len;

		so_fseek(stream, 0, SEEK_CUR);
	}

	//write character into buffer
	stream->buffer[stream->buff_cursor] = c;
	stream->buff_cursor++;
	stream->buff_len++;
	stream->last_operation = LAST_WRITE;

	//full buffer
	if (stream->buff_cursor == DEFAULT_BUFF_SIZE)
		so_fflush(stream);

	return c;
}


size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	//invalid stream
	if (stream == NULL)
		return 0;

	size_t		bytes_read = 0;
	int			c_getc;
	int			num_bytes = size*nmemb;
	char			*elements = (char *)malloc(num_bytes+1);

	memset(elements, '\0', num_bytes+1);

	while (bytes_read < num_bytes) {
		//so_fgetc will take a byte from the file
		c_getc = so_fgetc(stream);
		if (stream->end_file) {
			memcpy(ptr, elements, size*bytes_read);
			free(elements);
			return bytes_read/size;
		}
		if (c_getc == SO_EOF) {
			free(elements);
			return 0;
		}
		elements[bytes_read] = (unsigned char)c_getc;
		bytes_read++;
	}
	elements[bytes_read] = '\0';
	memcpy(ptr, elements, size*nmemb);
	free(elements);

	stream->last_operation = LAST_READ;

	return nmemb;
}


size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	//invalid stream
	if (stream == NULL)
		return 0;
	//doesn't have permision to write
	//invalid buffer
	if (stream->open_mode == OPEN_RD) {
		stream->error_occured = 1;
		return SO_EOF;
	}
	//the file was oppened with update (+)
	//the last operation was a read
	if (stream->last_operation == LAST_READ && stream->open_mode%2 == 1) {
		//move the file cursor to the effective place
		int	go_back = stream->buff_cursor-stream->buff_len;

		so_fseek(stream, go_back, SEEK_CUR);
	}

	size_t			bytes_write = 0;
	unsigned char		character;
	int				c_putc;

	while (bytes_write < nmemb*size) {
		character = ((unsigned char *)ptr)[bytes_write];
		c_putc = so_fputc((int)character, stream);
		if (c_putc == SO_EOF)
			return 0;
		bytes_write++;
	}
	return nmemb;

}


int so_fseek(SO_FILE *stream, long offset, int whence)
{
	long		real_offset = offset;
	int		state;

	//the last operation was a write
	if (stream->last_operation == LAST_WRITE) {
		if (so_fflush(stream) < 0) {
			stream->error_occured = 1;
			return SO_EOF;
		}
	}
	//the last operation was a read
	if (stream->last_operation == LAST_READ) {
		//reset buff status
		memset(stream->buffer, '\0', DEFAULT_BUFF_SIZE);
		stream->buff_cursor = 0;
		stream->buff_len = 0;
		if (stream->buff_cursor > 0)
			real_offset += stream->buff_cursor - DEFAULT_BUFF_SIZE;
	}

	if (whence == SEEK_CUR)
		state = lseek(stream->file_descriptor, real_offset, whence);
	else
		state = lseek(stream->file_descriptor, offset, whence);

	if (state == -1) {
		stream->error_occured = 1;
		return SO_EOF;
	}
	return 0;
}


long so_ftell(SO_FILE *stream)
{
	long		position;

	position = lseek(stream->file_descriptor, 0, SEEK_CUR);

	if (stream->last_operation == LAST_READ) {
		//move position backwards with the unread characters;
		position -= stream->buff_len - stream->buff_cursor;
	}

	if (stream->last_operation == LAST_WRITE) {
		//move position forward with the unwritten characters from buffer
		position += stream->buff_cursor;
	}

	return position;
}


int so_fflush(SO_FILE *stream)
{
	if (stream->last_operation == LAST_WRITE && stream->buff_cursor > 0) {
		//write the buffer content in
		int		written;

		written = write(stream->file_descriptor, stream->buffer, stream->buff_cursor);

		if (written < 0) {
			stream->error_occured = 1;
			memset(stream->buffer, '\0', DEFAULT_BUFF_SIZE);
			stream->buff_cursor = 0;
			stream->buff_len = 0;
			return SO_EOF;
		}
		while (written < stream->buff_len) {
			ssize_t result = write(stream->file_descriptor, stream->buffer + written, stream->buff_len - written);

			if (result == -1) {
				stream->error_occured = 1;
				memset(stream->buffer, '\0', DEFAULT_BUFF_SIZE);
				stream->buff_cursor = 0;
				stream->buff_len = 0;
				return SO_EOF;
			}
			written += result;
		}
		memset(stream->buffer, '\0', DEFAULT_BUFF_SIZE);
		stream->buff_cursor = 0;
		stream->buff_len = 0;
	}

	return 0;
}


int so_fileno(SO_FILE *stream)
{
	if (!stream)
		return -1;

	return stream->file_descriptor;
}


int so_feof(SO_FILE *stream)
{
	return stream->end_file;
}


int so_ferror(SO_FILE *stream)
{
	return stream->error_occured;
}


SO_FILE *so_popen(const char *command, const char *type)
{
	int		ret;
	int		pipe_fd[2];
	int		pid;

	// [0] - write end
	// [1] - read end
	ret = pipe(pipe_fd);
	if (ret < 0)
		return NULL;

	pid = fork();
	//Fork failed
	if (pid < 0) {
		close(pipe_fd[READ_END]);
		close(pipe_fd[WRITE_END]);
		return NULL;
	}
	if (pid == 0) {
		if (strcmp(type, "r") == 0) {
			//close read end
			close(pipe_fd[READ_END]);
			dup2(pipe_fd[WRITE_END], 1);
			close(pipe_fd[WRITE_END]);

		} else if (strcmp(type, "w") == 0) {
			//close write end
			close(pipe_fd[WRITE_END]);
			dup2(pipe_fd[READ_END], 0);
			close(pipe_fd[READ_END]);
		}

		execlp("sh", "sh", "-c", command, (char *)NULL);
		exit(-1);
	}
	if (pid > 0) {
		SO_FILE		*f;
		int			open_mode;

		f = (SO_FILE *)malloc(sizeof(SO_FILE));

		if (strcmp(type, "r") == 0) {
			//close write end
			close(pipe_fd[WRITE_END]);
			f->open_mode = OPEN_RD;
			f->file_descriptor = pipe_fd[READ_END];
		} else if (strcmp(type, "w") == 0) {
			//close read end
			close(pipe_fd[READ_END]);
			f->open_mode = OPEN_WR;
			f->file_descriptor = pipe_fd[WRITE_END];
		}
		f->buff_cursor = 0;
		f->last_operation = -1;
		f->error_occured = 0;
		f->associated_pid = pid;
		f->error_occured = 0;
		f->buff_len = 0;
		f->end_file = 0;
		f->buffer = NULL;
		f->buffer = (char *)calloc(DEFAULT_BUFF_SIZE, sizeof(char));
		if (f->buffer == NULL) {
			close(f->file_descriptor);
			free(f);
			f = NULL;
			return NULL;
		}

		return f;
	}

}


int so_pclose(SO_FILE *stream)
{
	int		status;
	pid_t	pid;
	pid_t	returned;

	pid = stream->associated_pid;
	so_fclose(stream);

	while (waitpid(pid, &status, 0) == -1) {
		status = -1;
		break;
	}

	return status;
}
