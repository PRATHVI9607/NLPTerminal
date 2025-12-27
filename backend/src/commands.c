#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/statvfs.h>

#include "commands.h"

#define BUFFER_SIZE 4096
#define BOOKMARK_FILE ".shell_bookmarks"
#define MAX_BOOKMARKS 50

// Global stats
static int total_commands = 0;

// Implementation of 'pwd' using getcwd system call
void do_pwd(char **args) {
    char cwd[1024];
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

    d = opendir(path);
    if (d) {
        printf("Name\t\tSize\n");
        printf("----\t\t----\n");
        while ((dir = readdir(d)) != NULL) {
            snprintf(full_path, sizeof(full_path), "%s/%s", path, dir->d_name);
            
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
    int fd = open(args[1], O_WRONLY | O_CREAT | O_NOCTTY | O_NONBLOCK, 0666);
    if (fd < 0) {
        perror("touch");
    } else {
        close(fd);
        printf("File '%s' touched/created.\n", args[1]);
    }
}

// Implementation of 'cat' using open/read/write system calls
void do_cat(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "cat: missing operand\n");
        return;
    }
    
    int fd = open(args[1], O_RDONLY);
    if (fd < 0) {
        perror("cat");
        return;
    }
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
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

    int src_fd = open(args[1], O_RDONLY);
    if (src_fd < 0) {
        perror("cp: source");
        return;
    }

    int dest_fd = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        perror("cp: destination");
        close(src_fd);
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
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
    
    if (rename(args[1], args[2]) != 0) {
        perror("mv");
    } else {
        printf("Moved '%s' to '%s'.\n", args[1], args[2]);
    }
}

// ============ CUSTOM COMMANDS ============

// Helper function for tree
static void print_tree_recursive(const char *path, int depth, const char *prefix) {
    DIR *d = opendir(path);
    if (!d) return;
    
    struct dirent *dir;
    struct stat file_stat;
    char full_path[1024];
    
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;
            
        snprintf(full_path, sizeof(full_path), "%s/%s", path, dir->d_name);
        
        for (int i = 0; i < depth; i++) printf("  ");
        printf("|-- %s", dir->d_name);
        
        if (stat(full_path, &file_stat) == 0 && S_ISDIR(file_stat.st_mode)) {
            printf("/\n");
            if (depth < 3) { // Limit depth to prevent too deep recursion
                print_tree_recursive(full_path, depth + 1, prefix);
            }
        } else {
            printf("\n");
        }
    }
    closedir(d);
}

void do_tree(char **args) {
    char *path = ".";
    if (args[1] != NULL) {
        path = args[1];
    }
    
    printf("%s\n", path);
    print_tree_recursive(path, 0, "");
}

void do_search(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "search: missing pattern\n");
        return;
    }
    
    char *pattern = args[1];
    DIR *d = opendir(".");
    if (!d) {
        perror("search");
        return;
    }
    
    struct dirent *dir;
    char buffer[BUFFER_SIZE];
    int found = 0;
    
    printf("Searching for '%s'...\n", pattern);
    
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        
        int fd = open(dir->d_name, O_RDONLY);
        if (fd < 0) continue;
        
        ssize_t bytes_read;
        int line_num = 1;
        int pos = 0;
        
        while ((bytes_read = read(fd, buffer + pos, 1)) > 0) {
            if (buffer[pos] == '\n') {
                buffer[pos] = '\0';
                if (strstr(buffer, pattern) != NULL) {
                    printf("%s:%d: %s\n", dir->d_name, line_num, buffer);
                    found++;
                }
                line_num++;
                pos = 0;
            } else {
                pos++;
                if (pos >= BUFFER_SIZE - 1) {
                    buffer[pos] = '\0';
                    if (strstr(buffer, pattern) != NULL) {
                        printf("%s:%d: %s...\n", dir->d_name, line_num, buffer);
                        found++;
                    }
                    pos = 0;
                }
            }
        }
        close(fd);
    }
    closedir(d);
    
    printf("Found %d matches.\n", found);
}

void do_backup(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "backup: missing file\n");
        return;
    }
    
    char backup_name[1024];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    
    snprintf(backup_name, sizeof(backup_name), "%s.backup_%04d%02d%02d_%02d%02d%02d",
             args[1],
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec);
    
    int src_fd = open(args[1], O_RDONLY);
    if (src_fd < 0) {
        perror("backup: source");
        return;
    }
    
    int dest_fd = open(backup_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        perror("backup: destination");
        close(src_fd);
        return;
    }
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        write(dest_fd, buffer, bytes_read);
    }
    
    close(src_fd);
    close(dest_fd);
    
    printf("Backup created: %s\n", backup_name);
}

void do_compare(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "compare: missing files\n");
        return;
    }
    
    int fd1 = open(args[1], O_RDONLY);
    int fd2 = open(args[2], O_RDONLY);
    
    if (fd1 < 0 || fd2 < 0) {
        perror("compare");
        if (fd1 >= 0) close(fd1);
        if (fd2 >= 0) close(fd2);
        return;
    }
    
    char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE];
    ssize_t read1, read2;
    int differences = 0;
    
    while (1) {
        read1 = read(fd1, buf1, sizeof(buf1));
        read2 = read(fd2, buf2, sizeof(buf2));
        
        if (read1 != read2 || memcmp(buf1, buf2, read1) != 0) {
            differences++;
        }
        
        if (read1 == 0 || read2 == 0) break;
    }
    
    close(fd1);
    close(fd2);
    
    if (differences == 0) {
        printf("Files are identical.\n");
    } else {
        printf("Files differ.\n");
    }
}

void do_stats(char **args) {
    total_commands++;
    printf("=== Shell Statistics ===\n");
    printf("Total commands executed: %d\n", total_commands);
    printf("Current directory: ");
    do_pwd(NULL);
}

void do_bookmark(char **args) {
    if (args[1] == NULL) {
        // List bookmarks
        int fd = open(BOOKMARK_FILE, O_RDONLY);
        if (fd < 0) {
            printf("No bookmarks saved.\n");
            return;
        }
        
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("=== Bookmarks ===\n%s", buffer);
        }
        close(fd);
    } else if (args[2] == NULL) {
        // Jump to bookmark
        int fd = open(BOOKMARK_FILE, O_RDONLY);
        if (fd < 0) {
            printf("No bookmarks found.\n");
            return;
        }
        
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            char *line = strtok(buffer, "\n");
            while (line) {
                char name[256], path[768];
                if (sscanf(line, "%s %s", name, path) == 2) {
                    if (strcmp(name, args[1]) == 0) {
                        if (chdir(path) == 0) {
                            printf("Jumped to: %s\n", path);
                        } else {
                            perror("bookmark");
                        }
                        return;
                    }
                }
                line = strtok(NULL, "\n");
            }
            printf("Bookmark '%s' not found.\n", args[1]);
        }
    } else {
        // Save bookmark
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("bookmark");
            return;
        }
        
        int fd = open(BOOKMARK_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
            perror("bookmark");
            return;
        }
        
        char entry[2048];
        snprintf(entry, sizeof(entry), "%s %s\n", args[1], args[2]);
        write(fd, entry, strlen(entry));
        close(fd);
        
        printf("Bookmark '%s' saved for %s\n", args[1], args[2]);
    }
}

void do_recent(char **args) {
    DIR *d = opendir(".");
    if (!d) {
        perror("recent");
        return;
    }
    
    struct dirent *dir;
    struct stat file_stat;
    time_t now = time(NULL);
    
    printf("=== Recently Modified Files (last 24 hours) ===\n");
    
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        
        if (stat(dir->d_name, &file_stat) == 0) {
            double diff = difftime(now, file_stat.st_mtime);
            if (diff < 86400) { // 24 hours
                printf("%s (%.0f seconds ago)\n", dir->d_name, diff);
            }
        }
    }
    closedir(d);
}

void do_bulk_rename(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "bulk_rename: usage: bulk_rename <pattern> <replacement>\n");
        return;
    }
    
    char *pattern = args[1];
    char *replacement = args[2];
    
    DIR *d = opendir(".");
    if (!d) {
        perror("bulk_rename");
        return;
    }
    
    struct dirent *dir;
    int renamed = 0;
    
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        
        char *found = strstr(dir->d_name, pattern);
        if (found) {
            char new_name[1024];
            int prefix_len = found - dir->d_name;
            
            strncpy(new_name, dir->d_name, prefix_len);
            new_name[prefix_len] = '\0';
            strcat(new_name, replacement);
            strcat(new_name, found + strlen(pattern));
            
            if (rename(dir->d_name, new_name) == 0) {
                printf("Renamed: %s -> %s\n", dir->d_name, new_name);
                renamed++;
            }
        }
    }
    closedir(d);
    
    printf("Renamed %d files.\n", renamed);
}

// System Resource Monitor - Linux only
// For full system monitor, use sysmon_advanced.c
void do_sysmon(char **args) {
    (void)args;
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                    SYSTEM RESOURCE MONITOR                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");

    // CPU Information
    printf("┌─ CPU Information ───────────────────────────────────────────┐\n");
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    int cores = 0;
    if (cpuinfo) {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (strncmp(line, "processor", 9) == 0) cores++;
        }
        fclose(cpuinfo);
        printf("│ Processors: %d cores\n", cores);
    }
    
    FILE *stat = fopen("/proc/stat", "r");
    if (stat) {
        unsigned long long user, nice, system, idle, iowait, irq, softirq;
        char cpu[10];
        if (fscanf(stat, "%s %llu %llu %llu %llu %llu %llu %llu",
                   cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 8) {
            unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
            unsigned long long active = total - idle;
            double cpuUsage = (double)active / total * 100.0;
            printf("│ CPU Usage: %.1f%%\n", cpuUsage);
            int bars = (int)(cpuUsage / 5);
            printf("│ [");
            for (int i = 0; i < 20; i++) printf(i < bars ? "█" : "░");
            printf("]\n");
        }
        fclose(stat);
    }
    printf("└─────────────────────────────────────────────────────────────┘\n\n");
    
    // Memory Information
    printf("┌─ Memory Information ────────────────────────────────────────┐\n");
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        unsigned long memTotal = 0, memAvailable = 0;
        char line[256];
        while (fgets(line, sizeof(line), meminfo)) {
            sscanf(line, "MemTotal: %lu kB", &memTotal);
            sscanf(line, "MemAvailable: %lu kB", &memAvailable);
        }
        fclose(meminfo);
        double totalGB = memTotal / (1024.0 * 1024.0);
        double availGB = memAvailable / (1024.0 * 1024.0);
        double usedGB = totalGB - availGB;
        int usage = (int)((usedGB / totalGB) * 100);
        printf("│ Total: %.2f GB  Used: %.2f GB  Free: %.2f GB\n", totalGB, usedGB, availGB);
        printf("│ Usage: %d%%  [", usage);
        for (int i = 0; i < 20; i++) printf(i < usage/5 ? "█" : "░");
        printf("]\n");
    }
    printf("└─────────────────────────────────────────────────────────────┘\n\n");
    
    // Disk Information
    printf("┌─ Disk Information ──────────────────────────────────────────┐\n");
    FILE *mtab = fopen("/proc/mounts", "r");
    if (mtab) {
        char line[512], device[256], mountpoint[256], fstype[64];
        struct statvfs vfs;
        while (fgets(line, sizeof(line), mtab)) {
            if (sscanf(line, "%s %s %s", device, mountpoint, fstype) == 3) {
                if (strncmp(device, "/dev/", 5) == 0 && statvfs(mountpoint, &vfs) == 0) {
                    unsigned long long total = vfs.f_blocks * vfs.f_frsize;
                    unsigned long long used = total - vfs.f_bfree * vfs.f_frsize;
                    double totalGB = total / (1024.0 * 1024.0 * 1024.0);
                    double usedGB = used / (1024.0 * 1024.0 * 1024.0);
                    if (totalGB > 0.1)
                        printf("│ %s: %.1f/%.1f GB\n", mountpoint, usedGB, totalGB);
                }
            }
        }
        fclose(mtab);
    }
    printf("└─────────────────────────────────────────────────────────────┘\n\n");
    
    // Uptime
    printf("┌─ System Uptime ─────────────────────────────────────────────┐\n");
    FILE *up = fopen("/proc/uptime", "r");
    if (up) {
        double secs;
        if (fscanf(up, "%lf", &secs) == 1) {
            int d = (int)(secs / 86400), h = (int)((secs - d*86400) / 3600);
            int m = (int)((secs - d*86400 - h*3600) / 60);
            printf("│ Uptime: %d days, %d hours, %d minutes\n", d, h, m);
        }
        fclose(up);
    }
    printf("└─────────────────────────────────────────────────────────────┘\n");
}
