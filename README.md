# LIBSO_STDIO.SO
This project implements a dynamic library modeled after the `stdio` library for Linux. It functions almost identically to the original library, passing all tests conducted with the checker `check-lin`, available at https://github.com/davram-code/pso/tree/pso-2023-2024/assignments/2-stdio/checker-lin [95/95].

## Archive Files

The project archive contains:

- `README.md` - this file
- `Makefile` - Makefile for automating the build and clean processes on Linux
- `so_stdio.h` - the header of the library (provided)
- `so_stdio.c` - the source file (containing the definition of functions declared in .h)

## SO_FILE

The main structure of the library that assists in operating with opened files is `SO_FILE`. It contains several parameters, including:

- `char *buffer;` - for buffering
- `int file_descriptor;` - the file descriptor of the opened file
- `int last_operation;` - saves the last operation on the file content (read/write)
- `int buff_cursor;` - the index in the buffer where the next operation should start
- `int error_occurred;` - 0 by default, 1 if an error occurred
- `int open_mode;` - a code for the opening mode of the file
- `int buff_len;` - the current number of valid bytes in the buffer (relevant for read)
- `int associated_pid;` - the PID of the child process started and associated with `SO_FILE*` (relevant for `popen/pclose` only)
- `int end_file;` - indicates if the cursor is at the end of the file or not

## Flags

The implementation uses three sets of flags declared with `#define` at the beginning of `so_stdio.c`. They aim to make the code more manageable and reduce the chances of errors.

- `LAST_*` <2 flags> - codes for the type of the last operation (read/write)
- `OPEN_*` <6 flags> - codes for the opening mode of the file
- `*_END` <2 flags> - codes for working with anonymous pipes - the indexes of each end (read/write)

## Function Implementation

- `so_fopen`
  - Uses a cascade of if/else to set the correct flags for the `open` operation.
  - Allocates stream (SO_FILE*) and initializes all its parameters.
  - Returns the stream or NULL in case of an error.

- `so_fclose`
  - If the last operation was a write (`LAST_WRITE`), it saves the changes in the file.
  - Frees the buffer and stream.
  - Returns 0 in case of success or `SO_EOF` in case of an error.

- `so_fgetc`
  - If the last operation was a write, calls `so_fseek`.
  - If the buffer is empty, it tries to read from the file (`DEFAULT_SIZE_BUFFER`).
  - Updates `stream->buff_len` with the real number of bytes read.
  - If the read returns 0, then `stream->end_file` is set.
  - Sets the last operation to `LAST_READ`.
  - Returns the character from the current position (`stream->buff_cursor`) or `SO_EOF` in case of an error.

- `so_fputc`
  - Works similarly to `fgetc` but in reverse.
  - If the last operation was a read, it calls `so_fseek`.
  - Stores the given character at the current position in the buffer.
  - If the buffer is full, it saves the changes in the file (calls `write`).
  - If the call to `write` returns fewer characters than expected, it continues writing until all characters are written.
  - Sets the last operation to `LAST_WRITE`.
  - Returns the given character or `SO_EOF` in case of an error.

- `so_fread`
  - Uses the `so_fgetc` function in a loop.
  - The loop ends either when all the elements have been read, the cursor reaches the end of the file, or an error occurs.
  - Returns the actual number of read elements in `ptr` or 0 in case of an error/eof reached.

- `so_fwrite`
  - Uses the `so_fputc` function in a loop.
  - The loop ends when all elements (bytes of the elements) are written or when an error occurs.
  - Returns the actual number of written elements from `ptr` or 0 in case of an error.

- `so_fseek`
  - Looks at the last operation executed on the file.
  - If it was a read, it resets the buffer.
  - If it was a write, it flushes the buffer (`so_fflush`).
  - Then it calls `fseek` to move the cursor according to the user request.
  - Returns 0 by default and `SO_EOF` in case of an error.

- `so_ftell`
  - Gets the real position of the cursor in the file.
  - If the last operation was read, it subtracts the number of unread bytes from the buffer.
  - If the last operation was write, it adds the number of bytes written to the buffer.

- `so_fileno`
  - Returns the file descriptor (`stream->file_descriptor`).

- `so_feof`
  - Returns the state of EOF (`stream->end_file`).

- `so_ferror`
  - Returns the error state of the stream (`stream->error_occurred`).

- `so_popen`
  - Uses `fork()`, `execlp()`, and `dup2()`.
  - Creates a child process, redirects its input/output to an anonymous pipe (based on the type), and overwrites the image of the process with `sh -c command`.
  - Returns in the parent process the `SO_FILE*` for the created pipe, with the opposite end file descriptor than the child process uses, on which all operations possible on a regular file can be executed.

- `so_pclose`
  - Uses `so_fclose` to close the file.
  - Then waits until the child process ends (`waitpid`).
  - Returns the status with which the child process exited.

## Cleanup
To clean the files object files generated and erase the library (libso_stdio.so):
```
make clean
```
## Build
To generate the library and the necessary .o file:
```
make build
``` 
or
```
make
```
