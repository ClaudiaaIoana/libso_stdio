# LIBSO_STDIO.SO
In this project is implemented a dinamic library having as model the Operating System `stdio` library for Linux. It works allmost identically to the original library passing all the tests made with the checker `check-lin` available on https://github.com/davram-code/pso/tree/pso-2023-2024/assignments/2-stdio/checker-lin [95/95].

# Archive Files
The project archive contains:
* `README.md`   - this file
* `Makefile`    - Makefile for automating the build and clean processes on Linux
* `so_stdio.h`  - the header of the library (given)
* `so_stdio.c`  - the cource file (with the definition of the functions declared in .h)

# SO_FILE
The main structure of the library that helps operating with the opened files is SO_FILE.
It contains a series of parameters as follows:

* `char *buffer;`		- for the buffering 
* `int file_descriptor;`	- the file descriptor of the opened file
* `int last_operation;`	- saves the last operation on the file content (read/write)
* `int buff_cursor;`	- the index in buffer where the following operation should start
* `int error_occured;`	- 0 default & 1 if an error occured
* `int open_mode;`		- a code for the opening mode of the file
* `int buff_len;`		- the current number of valid bytes in the buffer (relevant for read) 
* `int associated_pid;`	- the pid of the child process started and asociated with SO_FILE* (relevant for poepen/pclose only)
* `int end_file;`		- indicates if the cursour is at the end of the file or not


# Flags
In the implementation are used 3 sets of flags declared with `#define` at the begining of so_stdio.c. They have the purpose of making the code more manageble and slim the chances of making errors.
* `LAST_*` <2 flags>	- codes for the type of the last operation (read/write)
* `OPEN_*` <6 flags>	- codes for the opening mode of the file
* `*_END`  <2 flags>	- codes for working with anonimous pipes - the indexes of each end (read/write)

# Function implementation
* `so_fopen`	- uses a cascade of if/else to set the correct flags to pass to the `open` operation
			- allocs the SO_FILE and initialises all its parameters
			- returns stream or NULL in case of error

* `so_fclose`	- if the last operation was a write (LAST_WRITE), it saves the changes in the file
			- free the buffer and stream
			- returns 0 or SO_EOF in case of error

* `so_fgetc`	- if the last operation was a write, calls so_fseek
			- if the buffer is empty then it tries to read from the file `DEFAULT_SIZE_BUFFER` 
			- puts in stream->buff_len the real number of bytes read
			- if the read return 0 then the stream->end_file is set
			- sets the last operation to LAST_READ
			- then returns the character from the current position (stream->buff_cursor) of SO_EOF in case of error

* `so_fputc` 	- works similarly with fgetc but in reverse
			- if the last operation was a read it call so_fseek
			- stores at the current position in buffer the given character
			- if the buffer is full it saves the changes in file (calls write)
			- in case the call of write returned less characters that expected we continue writing until all the characters are written
			-sets the last operation to LAST_WRITE
			- return the given character or SO_EOF in case of error
	
* `so_fread`	- uses so_fgetc function in a loop
			- the loop ends eather when all the elements have been read, when the cursor gets to the end of file or when an error was faced
			- returns the actual number of read elements in ptr or 0 in case of error/eof reached

* `so_fwrite`	- uses so_fputc function in a loop
			- the loop ends when all the elements (bytes of the elements) were written or when an error was faced
			- returns the actual number of written elements from ptr or 0 in case of error

* `so_fseek`	- looks at the last operation executed on the file
			- if it was a read then is resets the buffer
				-> if the fseek's whence is SEEK_CUR it subracts the bytes that weren't actually read from the buffer
			- if it was a write flushes the buffer (so_fflush)
			- then it call `fseek` to move the cursor acording with the user request
			- returns 0 default and SO_EOF in case of error

* `so_ftell`	- gets the real positon of the cursor in the file
			- if the last operation was read then it subtract the number of unread bytes from the buffer
			- if the last operation was write then it adds the number of bytes written in the buffer

* `so_fileno`	- returns the file descriptor (stream->file_descriptor)

* `so_feof`	- returns the state of eof (stream->end_file)

* `so_ferror`	- returns the error state of the stream (stream->error_occured)

* `so_popen`	- uses the `fork()`, `execlp()` and `dup2()`
			- creates a child process, redirects its input/output to an anonimous pipe (based on the type) and overwrites the image of the process with `sh -c command`
			- returns in the parent process the SO_FILE* for the created pipe, with the opposite end file descriptor than the child process uses, on which can be executed all the normal operations possible on a regular file

* `so_pclose`	- uses `so_fclose` to close the file 
			- then waits until the child process ends  (`waitpid`)
			- returns the status with which the child process exited

# Cleanup
To clean the files object files generated and erase the library (libso_stdio.so):
```
make clean
```
# Build
To generate the library and the necessary .o file:
```
make build
``` 
or
```
make
```