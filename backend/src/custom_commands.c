/**
 * Custom Commands - Linux Only
 * Unique commands using system calls
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <stdint.h>
#include "custom_commands.h"

#define BUFFER_SIZE 4096

// Hash function
static unsigned long file_hash(const char *filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return 0;
    unsigned long hash = 5381;
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++)
            hash = ((hash << 5) + hash) + (unsigned char)buf[i];
    }
    close(fd);
    return hash;
}

static void format_size(long long bytes, char *out, size_t sz) {
    const char *u[] = {"B", "KB", "MB", "GB"};
    int i = 0;
    double s = bytes;
    while (s >= 1024 && i < 3) { s /= 1024; i++; }
    snprintf(out, sz, "%.1f %s", s, u[i]);
}

// fileinfo - detailed file info
void do_fileinfo(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: fileinfo <file>\n"); return; }
    struct stat st;
    if (stat(args[1], &st) != 0) { perror("fileinfo"); return; }
    
    char sz[32]; format_size(st.st_size, sz, sizeof(sz));
    printf("\n=== FILE INFO: %s ===\n", args[1]);
    printf("Size: %lld bytes (%s)\n", (long long)st.st_size, sz);
    printf("Type: %s\n", S_ISDIR(st.st_mode) ? "Directory" : S_ISREG(st.st_mode) ? "File" : "Other");
    printf("Mode: %o\n", st.st_mode & 0777);
    printf("Modified: %s", ctime(&st.st_mtime));
    printf("Inode: %lu\n", st.st_ino);
    if (S_ISREG(st.st_mode)) printf("Hash: %lx\n", file_hash(args[1]));
    printf("\n");
}

// hexdump - show file in hex
void do_hexdump(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: hexdump <file> [bytes]\n"); return; }
    int fd = open(args[1], O_RDONLY);
    if (fd < 0) { perror("hexdump"); return; }
    
    int max = args[2] ? atoi(args[2]) : 256;
    unsigned char buf[16];
    ssize_t n;
    int offset = 0;
    
    while ((n = read(fd, buf, 16)) > 0 && offset < max) {
        printf("%08x  ", offset);
        for (int i = 0; i < 16; i++) {
            if (i < n) printf("%02x ", buf[i]); else printf("   ");
            if (i == 7) printf(" ");
        }
        printf(" |");
        for (int i = 0; i < n; i++)
            printf("%c", (buf[i] >= 32 && buf[i] < 127) ? buf[i] : '.');
        printf("|\n");
        offset += n;
    }
    close(fd);
}

// duplicate - find duplicate files
void do_duplicate(char **args) {
    const char *dir = args[1] ? args[1] : ".";
    DIR *d = opendir(dir);
    if (!d) { perror("duplicate"); return; }
    
    struct { char name[256]; unsigned long hash; } files[500];
    int count = 0;
    struct dirent *ent;
    
    while ((ent = readdir(d)) && count < 500) {
        if (ent->d_name[0] == '.') continue;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            strncpy(files[count].name, path, 255);
            files[count].hash = file_hash(path);
            count++;
        }
    }
    closedir(d);
    
    printf("Checking %d files for duplicates...\n", count);
    int found = 0;
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (files[i].hash == files[j].hash && files[i].hash != 0) {
                printf("DUPLICATE: %s <-> %s\n", files[i].name, files[j].name);
                found++;
            }
        }
    }
    if (!found) printf("No duplicates found.\n");
}

// encrypt/decrypt - simple XOR
void do_encrypt(char **args) {
    if (!args[1] || !args[2]) { fprintf(stderr, "Usage: encrypt <file> <key>\n"); return; }
    int fd = open(args[1], O_RDONLY);
    if (fd < 0) { perror("encrypt"); return; }
    
    struct stat st; 
    if (fstat(fd, &st) != 0) { perror("encrypt"); close(fd); return; }
    
    char *data = malloc(st.st_size);
    if (!data) { fprintf(stderr, "encrypt: memory allocation failed\n"); close(fd); return; }
    
    ssize_t bytes_read = read(fd, data, st.st_size);
    close(fd);
    
    if (bytes_read != st.st_size) {
        fprintf(stderr, "encrypt: read error\n");
        free(data);
        return;
    }
    
    int klen = strlen(args[2]);
    for (off_t i = 0; i < st.st_size; i++) data[i] ^= args[2][i % klen];
    
    char out[512]; snprintf(out, sizeof(out), "%s.enc", args[1]);
    fd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("encrypt"); free(data); return; }
    
    write(fd, data, st.st_size);
    close(fd);
    free(data);
    printf("Encrypted: %s -> %s\n", args[1], out);
}

void do_decrypt(char **args) {
    if (!args[1] || !args[2]) { fprintf(stderr, "Usage: decrypt <file> <key>\n"); return; }
    do_encrypt(args);  // XOR is symmetric
    printf("(XOR decryption same as encryption)\n");
}

// sizeof - total size of files matching pattern
void do_sizeof(char **args) {
    const char *pattern = args[1] ? args[1] : "*";
    DIR *d = opendir(".");
    if (!d) return;
    
    long long total = 0;
    int count = 0;
    struct dirent *ent;
    
    while ((ent = readdir(d))) {
        if (pattern[0] == '*' || strstr(ent->d_name, pattern + 1)) {
            struct stat st;
            if (stat(ent->d_name, &st) == 0 && S_ISREG(st.st_mode)) {
                total += st.st_size;
                count++;
            }
        }
    }
    closedir(d);
    
    char sz[32]; format_size(total, sz, sizeof(sz));
    printf("%d files, total: %s\n", count, sz);
}

// age - find files by age
void do_age(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: age <days> [older|newer]\n"); return; }
    int days = atoi(args[1]);
    int older = !args[2] || args[2][0] != 'n';
    time_t cutoff = time(NULL) - days * 86400;
    
    DIR *d = opendir(".");
    if (!d) return;
    struct dirent *ent;
    
    while ((ent = readdir(d))) {
        struct stat st;
        if (stat(ent->d_name, &st) == 0) {
            int match = older ? (st.st_mtime < cutoff) : (st.st_mtime > cutoff);
            if (match) printf("%s\n", ent->d_name);
        }
    }
    closedir(d);
}

// freq - word frequency
void do_freq(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: freq <file> [top_n]\n"); return; }
    FILE *fp = fopen(args[1], "r");
    if (!fp) { perror("freq"); return; }
    
    struct { char word[64]; int count; } words[1000];
    int wc = 0;
    char buf[256];
    
    while (fscanf(fp, "%255s", buf) == 1 && wc < 1000) {
        // Clean word
        char *p = buf;
        while (*p) { *p = tolower(*p); p++; }
        
        int found = -1;
        for (int i = 0; i < wc; i++) {
            if (strcmp(words[i].word, buf) == 0) { found = i; break; }
        }
        if (found >= 0) words[found].count++;
        else { strncpy(words[wc].word, buf, 63); words[wc].count = 1; wc++; }
    }
    fclose(fp);
    
    // Sort by count (simple bubble)
    for (int i = 0; i < wc - 1; i++) {
        for (int j = i + 1; j < wc; j++) {
            if (words[j].count > words[i].count) {
                char tw[64]; strcpy(tw, words[i].word);
                int tc = words[i].count;
                strcpy(words[i].word, words[j].word);
                words[i].count = words[j].count;
                strcpy(words[j].word, tw);
                words[j].count = tc;
            }
        }
    }
    
    int top = args[2] ? atoi(args[2]) : 10;
    printf("Top %d words:\n", top);
    for (int i = 0; i < top && i < wc; i++)
        printf("%4d: %s\n", words[i].count, words[i].word);
}

// lines - line/word/char count
void do_lines(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: lines <file>\n"); return; }
    int fd = open(args[1], O_RDONLY);
    if (fd < 0) { perror("lines"); return; }
    
    int lines = 0, words = 0, chars = 0, in_word = 0;
    char buf[4096];
    ssize_t n;
    
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            chars++;
            if (buf[i] == '\n') lines++;
            if (isspace(buf[i])) in_word = 0;
            else if (!in_word) { words++; in_word = 1; }
        }
    }
    close(fd);
    printf("Lines: %d  Words: %d  Chars: %d\n", lines, words, chars);
}

// quicknote - simple notes
void do_quicknote(char **args) {
    const char *file = ".quicknotes";
    
    if (!args[1] || strcmp(args[1], "list") == 0) {
        FILE *fp = fopen(file, "r");
        if (!fp) { printf("No notes.\n"); return; }
        char line[256];
        int n = 1;
        while (fgets(line, sizeof(line), fp)) printf("%d. %s", n++, line);
        fclose(fp);
    } else if (strcmp(args[1], "add") == 0 && args[2]) {
        FILE *fp = fopen(file, "a");
        fprintf(fp, "%s\n", args[2]);
        fclose(fp);
        printf("Note added.\n");
    } else if (strcmp(args[1], "clear") == 0) {
        unlink(file);
        printf("Notes cleared.\n");
    } else {
        printf("Usage: quicknote [list|add <text>|clear]\n");
    }
}

// calc - simple calculator
void do_calc(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: calc <expression>\n"); return; }
    
    // Build expression string
    char expr[256] = "";
    for (int i = 1; args[i]; i++) {
        if (strlen(expr) + strlen(args[i]) + 1 < sizeof(expr)) {
            strcat(expr, args[i]);
            if (args[i+1]) strcat(expr, " ");
        }
    }
    
    // Remove spaces for parsing
    char clean[256] = "";
    int j = 0;
    for (int i = 0; expr[i] && j < 255; i++) {
        if (!isspace((unsigned char)expr[i])) clean[j++] = expr[i];
    }
    clean[j] = '\0';
    
    // Parse: num op num
    double a, b, result;
    char op;
    int parsed = 0;
    
    // Try to find operator position
    for (int i = 1; clean[i]; i++) {
        if (clean[i] == '+' || clean[i] == '-' || clean[i] == '*' || clean[i] == '/' || clean[i] == '^') {
            char left[128] = "", right[128] = "";
            strncpy(left, clean, i);
            strcpy(right, clean + i + 1);
            a = atof(left);
            b = atof(right);
            op = clean[i];
            parsed = 1;
            break;
        }
    }
    
    if (parsed) {
        switch (op) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/': result = b != 0 ? a / b : 0; if (b == 0) { printf("Error: Division by zero\n"); return; } break;
            case '^': result = 1; for (int i = 0; i < (int)b; i++) result *= a; break;
            default: printf("Unknown operator: %c\n", op); return;
        }
        printf("= %.6g\n", result);
    } else {
        printf("Format: calc <num> <op> <num> (e.g., calc 2 + 3)\n");
    }
}

// head - first n lines
void do_head(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: head <file> [n]\n"); return; }
    FILE *fp = fopen(args[1], "r");
    if (!fp) { perror("head"); return; }
    
    int n = args[2] ? atoi(args[2]) : 10;
    char line[1024];
    while (n-- > 0 && fgets(line, sizeof(line), fp)) printf("%s", line);
    fclose(fp);
}

// tail - last n lines
void do_tail(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: tail <file> [n]\n"); return; }
    FILE *fp = fopen(args[1], "r");
    if (!fp) { perror("tail"); return; }
    
    int n = args[2] ? atoi(args[2]) : 10;
    char lines[100][1024];
    int count = 0, idx = 0;
    
    while (fgets(lines[idx], 1024, fp)) {
        idx = (idx + 1) % 100;
        if (count < 100) count++;
    }
    fclose(fp);
    
    int start = (count < n) ? 0 : (idx - n + 100) % 100;
    int show = (count < n) ? count : n;
    for (int i = 0; i < show; i++)
        printf("%s", lines[(start + i) % 100]);
}

// wc - word count
void do_wc(char **args) { do_lines(args); }

// grep - simple search
void do_grep(char **args) {
    if (!args[1] || !args[2]) { fprintf(stderr, "Usage: grep <pattern> <file>\n"); return; }
    FILE *fp = fopen(args[2], "r");
    if (!fp) { perror("grep"); return; }
    
    char line[1024];
    int n = 1;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, args[1])) printf("%d: %s", n, line);
        n++;
    }
    fclose(fp);
}

// sort - sort file lines
void do_sort(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: sort <file>\n"); return; }
    FILE *fp = fopen(args[1], "r");
    if (!fp) { perror("sort"); return; }
    
    char *lines[10000];
    int count = 0;
    char buf[1024];
    
    while (fgets(buf, sizeof(buf), fp) && count < 10000) {
        lines[count] = strdup(buf);
        if (!lines[count]) {
            fprintf(stderr, "sort: memory allocation failed\n");
            for (int i = 0; i < count; i++) free(lines[i]);
            fclose(fp);
            return;
        }
        count++;
    }
    fclose(fp);
    
    // Quick sort comparison using qsort for better efficiency
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(lines[i], lines[j]) > 0) {
                char *t = lines[i]; lines[i] = lines[j]; lines[j] = t;
            }
        }
    }
    
    for (int i = 0; i < count; i++) { printf("%s", lines[i]); free(lines[i]); }
}

// uniq - remove adjacent duplicates
void do_uniq(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: uniq <file>\n"); return; }
    FILE *fp = fopen(args[1], "r");
    if (!fp) { perror("uniq"); return; }
    
    char prev[1024] = "", line[1024];
    while (fgets(line, sizeof(line), fp)) {
        if (strcmp(line, prev) != 0) {
            printf("%s", line);
            strcpy(prev, line);
        }
    }
    fclose(fp);
}

// rev - reverse lines
void do_rev(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: rev <file>\n"); return; }
    FILE *fp = fopen(args[1], "r");
    if (!fp) { perror("rev"); return; }
    
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        int len = strlen(line);
        if (line[len-1] == '\n') len--;
        for (int i = len - 1; i >= 0; i--) putchar(line[i]);
        putchar('\n');
    }
    fclose(fp);
}

// clear
void do_clear(char **args) { (void)args; printf("\033[2J\033[H"); }

// date
void do_date(char **args) {
    (void)args;
    time_t t = time(NULL);
    printf("%s", ctime(&t));
}

// whoami
void do_whoami(char **args) {
    (void)args;
    struct passwd *pw = getpwuid(getuid());
    printf("%s\n", pw ? pw->pw_name : "unknown");
}

// hostname
void do_hostname(char **args) {
    (void)args;
    char buf[256];
    gethostname(buf, sizeof(buf));
    printf("%s\n", buf);
}

// uptime
void do_uptime(char **args) {
    (void)args;
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp) {
        double up;
        fscanf(fp, "%lf", &up);
        fclose(fp);
        int d = (int)up / 86400;
        int h = ((int)up % 86400) / 3600;
        int m = ((int)up % 3600) / 60;
        printf("up %d days, %02d:%02d\n", d, h, m);
    }
}

// df - disk free
void do_df(char **args) {
    (void)args;
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) return;
    
    printf("%-20s %10s %10s %10s %5s\n", "Filesystem", "Size", "Used", "Avail", "Use%");
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char dev[128], mnt[128], fs[32];
        if (sscanf(line, "%s %s %s", dev, mnt, fs) == 3 && strncmp(dev, "/dev/", 5) == 0) {
            struct statvfs st;
            if (statvfs(mnt, &st) == 0 && st.f_blocks > 0) {
                uint64_t total = st.f_blocks * st.f_frsize;
                uint64_t free = st.f_bfree * st.f_frsize;
                uint64_t used = total - free;
                char ts[16], us[16], fs2[16];
                format_size(total, ts, 16);
                format_size(used, us, 16);
                format_size(free, fs2, 16);
                printf("%-20s %10s %10s %10s %4.0f%%\n", mnt, ts, us, fs2, 100.0*used/total);
            }
        }
    }
    fclose(fp);
}

// ps - process list
void do_ps(char **args) {
    (void)args;
    printf("%-8s %-25s\n", "PID", "COMMAND");
    DIR *d = opendir("/proc");
    if (!d) return;
    
    struct dirent *ent;
    while ((ent = readdir(d))) {
        int pid = atoi(ent->d_name);
        if (pid > 0) {
            char path[64], name[256] = "";
            snprintf(path, sizeof(path), "/proc/%d/comm", pid);
            FILE *fp = fopen(path, "r");
            if (fp) { fgets(name, sizeof(name), fp); fclose(fp); }
            char *nl = strchr(name, '\n'); if (nl) *nl = 0;
            printf("%-8d %-25s\n", pid, name);
        }
    }
    closedir(d);
}

// kill
void do_kill(char **args) {
    if (!args[1]) { fprintf(stderr, "Usage: kill <pid>\n"); return; }
    int pid = atoi(args[1]);
    if (kill(pid, SIGTERM) == 0) printf("Killed %d\n", pid);
    else perror("kill");
}
