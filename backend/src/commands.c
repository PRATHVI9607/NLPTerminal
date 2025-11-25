#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include "commands.h"

#define BUFFER_SIZE 4096

// Implementation of 'pwd' using getcwd system call
void do_pwd(char **args) {
    char cwd[1024];
    // getcwd is a system call wrapper that gets the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
}

// Implementation of 'ls' using opendir/readdir system calls
void do_ls(char **args) {
    char *path = ".";
    if (args[1] != NULL) {
        path = args[1];
    }

    DIR *d;
    struct dirent *dir;
    struct stat file_stat;
    char full_path[1024];

    // opendir is a syscall wrapper to open a directory stream
    d = opendir(path);
    if (d) {
        printf("Name\t\tSize\n");
        printf("----\t\t----\n");
        // readdir reads the next directory entry
        while ((dir = readdir(d)) != NULL) {
            // Construct full path for stat
            snprintf(full_path, sizeof(full_path), "%s/%s", path, dir->d_name);
            
            // stat syscall gets file metadata (like size)
            if (stat(full_path, &file_stat) == 0) {
                printf("%-15s\t%ld bytes\n", dir->d_name, file_stat.st_size);
            } else {
                printf("%s\n", dir->d_name);
            }
        }
        closedir(d);
    } else {
        perror("ls");
    }
}

// Implementation of 'mkdir' using mkdir system call
void do_mkdir(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "mkdir: missing operand\n");
        return;
    }
    // mkdir syscall creates a directory with specified permissions (0755)
    if (mkdir(args[1], 0755) != 0) {
        perror("mkdir");
    } else {
        printf("Directory '%s' created.\n", args[1]);
    }
}

// Implementation of 'rmdir' using rmdir system call
void do_rmdir(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "rmdir: missing operand\n");
        return;
    }
    // rmdir syscall removes a directory
    if (rmdir(args[1]) != 0) {
        perror("rmdir");
    } else {
        printf("Directory '%s' removed.\n", args[1]);
    }
}

// Implementation of 'rm' using unlink system call
void do_rm(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "rm: missing operand\n");
        return;
    }
    // unlink syscall deletes a name from the filesystem (deletes file)
    if (unlink(args[1]) != 0) {
        perror("rm");
    } else {
        printf("File '%s' removed.\n", args[1]);
    }
}

// Implementation of 'touch' using open system call
void do_touch(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "touch: missing operand\n");
        return;
    }
    // open with O_CREAT creates the file if it doesn't exist
    // 0666 sets read/write permissions
    int fd = open(args[1], O_WRONLY | O_CREAT | O_NOCTTY | O_NONBLOCK, 0666);
    if (fd < 0) {
        perror("touch");
    } else {
        close(fd); // close syscall
        printf("File '%s' touched/created.\n", args[1]);
    }
}

// Implementation of 'cat' using open/read/write system calls
void do_cat(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "cat: missing operand\n");
        return;
    }
    
    // open syscall to open file for reading
    int fd = open(args[1], O_RDONLY);
    if (fd < 0) {
        perror("cat");
        return;
    }
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // read syscall reads bytes into buffer
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        // write syscall writes bytes to STDOUT (file descriptor 1)
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    
    close(fd);
}

void do_echo(char **args) {
    int i = 1;
    while (args[i] != NULL) {
        write(STDOUT_FILENO, args[i], strlen(args[i]));
        if (args[i+1] != NULL) write(STDOUT_FILENO, " ", 1);
        i++;
    }
    write(STDOUT_FILENO, "\n", 1);
}

// Implementation of 'cp' using open/read/write system calls
void do_cp(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "cp: missing source or destination\n");
        return;
    }

    // Open source file
    int src_fd = open(args[1], O_RDONLY);
    if (src_fd < 0) {
        perror("cp: source");
        return;
    }

    // Open/Create destination file
    // O_TRUNC clears the file if it exists
    int dest_fd = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        perror("cp: destination");
        close(src_fd);
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // Copy loop: read from source, write to dest
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            perror("cp: write error");
            break;
        }
    }

    printf("Copied '%s' to '%s'.\n", args[1], args[2]);

    close(src_fd);
    close(dest_fd);
}

// Implementation of 'mv' using rename system call
void do_mv(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "mv: missing source or destination\n");
        return;
    }
    
    // rename syscall moves/renames a file
    if (rename(args[1], args[2]) != 0) {
        perror("mv");
    } else {
        printf("Moved '%s' to '%s'.\n", args[1], args[2]);
    }
}
