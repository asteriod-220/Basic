A lightweight, unbuffered command-line hex dumper built entirely using raw POSIX Linux system calls.
This tool reads any file byte-by-byte and displays its contents in a formatted 16-byte hexadecimal grid.

- The program was built using the following headers for the following reason:
-   <unistd.h> for system calls such as open() read() close()
-   <stdlib.h> for malloc() free() and exit()
-   <fcntl.h>  to open file in read only mode using O_RDONLY
-   <string.g> to get the exact bytes of items
    <sys/stat.h> to use predefined struct stat to get the file size

# Compilation and Usage

* Compile the source code using gcc
* gcc hex_dump.c -o output


* Execute the compiled binary using ./
* ./output file

** This program is still under development and there will be improvements made. **
