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
#include <math.h>
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
            memset(files[count].name, 0, 256);
            memcpy(files[count].name, path, strlen(path) < 255 ? strlen(path) : 255);
            files[count].hash = file_hash(path);
            count++;
        }
    }
    closedir(d);
    
    if (count == 0) {
        printf("No files to check.\n");
        return;
    }
    
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
    
    if (strlen(args[2]) == 0) {
        fprintf(stderr, "encrypt: key cannot be empty\n");
        return;
    }
    
    int fd = open(args[1], O_RDONLY);
    if (fd < 0) { perror("encrypt"); return; }
    
    struct stat st; 
    if (fstat(fd, &st) != 0) { perror("encrypt"); close(fd); return; }
    
    if (st.st_size == 0) {
        fprintf(stderr, "encrypt: file is empty\n");
        close(fd);
        return;
    }
    
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
    
    ssize_t written = write(fd, data, st.st_size);
    close(fd);
    free(data);
    
    if (written == st.st_size) {
        printf("Encrypted: %s -> %s\n", args[1], out);
    } else {
        fprintf(stderr, "encrypt: write error\n");
    }
}

void do_decrypt(char **args) {
    if (!args[1] || !args[2]) { fprintf(stderr, "Usage: decrypt <file> <key>\n"); return; }
    
    int fd = open(args[1], O_RDONLY);
    if (fd < 0) { perror("decrypt"); return; }
    
    struct stat st; 
    if (fstat(fd, &st) != 0) { perror("decrypt"); close(fd); return; }
    
    char *data = malloc(st.st_size);
    if (!data) { fprintf(stderr, "decrypt: memory allocation failed\n"); close(fd); return; }
    
    ssize_t bytes_read = read(fd, data, st.st_size);
    close(fd);
    
    if (bytes_read != st.st_size) {
        fprintf(stderr, "decrypt: read error\n");
        free(data);
        return;
    }
    
    // XOR is symmetric - same operation as encryption
    int klen = strlen(args[2]);
    for (off_t i = 0; i < st.st_size; i++) data[i] ^= args[2][i % klen];
    
    // Remove .enc extension if present, otherwise add .dec
    char out[512];
    char *ext = strstr(args[1], ".enc");
    if (ext && ext[4] == '\0') {
        // Remove .enc extension
        size_t len = ext - args[1];
        strncpy(out, args[1], len);
        out[len] = '\0';
    } else {
        snprintf(out, sizeof(out), "%s.dec", args[1]);
    }
    
    fd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("decrypt"); free(data); return; }
    
    ssize_t written = write(fd, data, st.st_size);
    close(fd);
    free(data);
    
    if (written == st.st_size) {
        printf("Decrypted: %s -> %s\n", args[1], out);
    } else {
        fprintf(stderr, "decrypt: write error\n");
    }
}

// sizeof - total size of files matching pattern
void do_sizeof(char **args) {
    const char *pattern = args[1] ? args[1] : "*";
    DIR *d = opendir(".");
    if (!d) { perror("sizeof"); return; }
    
    long long total = 0;
    int count = 0;
    struct dirent *ent;
    
    while ((ent = readdir(d))) {
        if (ent->d_name[0] == '.') continue;  // Skip hidden files
        
        // Match pattern: * = all, otherwise substring match
        int matches = 0;
        if (strcmp(pattern, "*") == 0) {
            matches = 1;
        } else if (pattern[0] == '*' && strlen(pattern) > 1) {
            // *pattern - ends with
            matches = (strstr(ent->d_name, pattern + 1) != NULL);
        } else if (pattern[strlen(pattern)-1] == '*' && strlen(pattern) > 1) {
            // pattern* - starts with
            matches = (strncmp(ent->d_name, pattern, strlen(pattern) - 1) == 0);
        } else {
            // Exact substring match
            matches = (strstr(ent->d_name, pattern) != NULL);
        }
        
        if (matches) {
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
        else { memset(words[wc].word, 0, 64); memcpy(words[wc].word, buf, strlen(buf) < 63 ? strlen(buf) : 63); words[wc].count = 1; wc++; }
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

// calc - helper: operator precedence
static int calc_prec(char op) {
    if (op == '^') return 3;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

// calc - full expression calculator using shunting-yard algorithm
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
    
    // Stacks for shunting-yard
    double numStack[64];
    int numTop = -1;
    char opStack[64];
    int opTop = -1;
    
    char *p = expr;
    int expect_num = 1;  // For handling unary minus
    
    while (*p) {
        // Skip whitespace
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        
        // Number (including negative)
        if (isdigit((unsigned char)*p) || (*p == '.' && isdigit((unsigned char)*(p+1))) ||
            (expect_num && *p == '-' && (isdigit((unsigned char)*(p+1)) || *(p+1) == '.'))) {
            char *end;
            double num = strtod(p, &end);
            numStack[++numTop] = num;
            p = end;
            expect_num = 0;
            continue;
        }
        
        // Left paren
        if (*p == '(') {
            opStack[++opTop] = '(';
            p++;
            expect_num = 1;
            continue;
        }
        
        // Right paren
        if (*p == ')') {
            while (opTop >= 0 && opStack[opTop] != '(') {
                // Apply operator
                if (numTop >= 1) {
                    double b = numStack[numTop--];
                    double a = numStack[numTop--];
                    char op = opStack[opTop--];
                    double r = 0;
                    switch (op) {
                        case '+': r = a + b; break;
                        case '-': r = a - b; break;
                        case '*': r = a * b; break;
                        case '/': r = (b != 0) ? a / b : 0; break;
                        case '%': r = (b != 0) ? fmod(a, b) : 0; break;
                        case '^': r = pow(a, b); break;
                    }
                    numStack[++numTop] = r;
                }
            }
            if (opTop >= 0) opTop--;  // Remove '('
            p++;
            expect_num = 0;
            continue;
        }
        
        // Operator
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '^' || *p == '%') {
            char op = *p;
            int right_assoc = (op == '^');
            
            while (opTop >= 0 && opStack[opTop] != '(' &&
                   (calc_prec(opStack[opTop]) > calc_prec(op) || 
                    (calc_prec(opStack[opTop]) == calc_prec(op) && !right_assoc))) {
                // Apply operator
                if (numTop >= 1) {
                    double b = numStack[numTop--];
                    double a = numStack[numTop--];
                    char top_op = opStack[opTop--];
                    double r = 0;
                    switch (top_op) {
                        case '+': r = a + b; break;
                        case '-': r = a - b; break;
                        case '*': r = a * b; break;
                        case '/': r = (b != 0) ? a / b : 0; break;
                        case '%': r = (b != 0) ? fmod(a, b) : 0; break;
                        case '^': r = pow(a, b); break;
                    }
                    numStack[++numTop] = r;
                }
            }
            opStack[++opTop] = op;
            p++;
            expect_num = 1;
            continue;
        }
        
        // Unknown char, skip
        p++;
    }
    
    // Apply remaining operators
    while (opTop >= 0) {
        if (numTop >= 1) {
            double b = numStack[numTop--];
            double a = numStack[numTop--];
            char op = opStack[opTop--];
            double r = 0;
            switch (op) {
                case '+': r = a + b; break;
                case '-': r = a - b; break;
                case '*': r = a * b; break;
                case '/': r = (b != 0) ? a / b : 0; break;
                case '%': r = (b != 0) ? fmod(a, b) : 0; break;
                case '^': r = pow(a, b); break;
            }
            numStack[++numTop] = r;
        } else {
            break;
        }
    }
    
    if (numTop >= 0) {
        printf("= %.6g\n", numStack[0]);
    } else {
        printf("Error: Invalid expression\n");
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
void do_clear(char **args) { 
    (void)args; 
    // Send clear signal for GUI (no ANSI codes, just the marker)
    printf("CLEAR_SCREEN\n");
    fflush(stdout);
}

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

// uniquecmds - show commands unique to NLP Terminal (not in standard UNIX)
void do_uniquecmds(char **args) {
    (void)args;
    printf("\n");
    printf("\033[32m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("  \033[1;36mUnique Commands\033[0m  —  Not in standard UNIX\n");
    printf("\033[32m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
    
    printf("\033[33m  Live OS Monitoring (opens real-time windows):\033[0m\n");
    printf("    \033[32mpaging\033[0m        Page fault & swap stats from /proc/vmstat\n");
    printf("    \033[32mvmstat\033[0m        Virtual memory statistics\n");
    printf("    \033[32mproclist\033[0m      Process list (custom top)\n");
    printf("    \033[32mproctop\033[0m       Top N processes by memory\n");
    printf("    \033[32mzoneinfo\033[0m      Memory zone info (NUMA)\n");
    printf("    \033[32mconnections\033[0m   Active network connections\n");
    printf("    \033[32mopenfiles\033[0m     Open file descriptors\n");
    printf("    \033[32msockets\033[0m       Socket statistics\n\n");
    
    printf("\033[33m  File Utilities:\033[0m\n");
    printf("    \033[32mfileinfo <f>\033[0m  Detailed file info (size, hash, perms)\n");
    printf("    \033[32mhexdump <f>\033[0m   Hex view of file\n");
    printf("    \033[32mduplicate\033[0m     Find duplicates by hash\n");
    printf("    \033[32mfreq <file>\033[0m   Word frequency analysis\n");
    printf("    \033[32mlines <file>\033[0m  Line/word/char stats\n");
    printf("    \033[32mage <days>\033[0m    Find files by age\n");
    printf("    \033[32msizeof <pat>\033[0m  Size of matching files\n\n");
    
    printf("\033[33m  Inter-Process Communication:\033[0m\n");
    printf("    \033[32mmypid\033[0m              This terminal's PID\n");
    printf("    \033[32mterminals\033[0m          List all terminal tabs\n");
    printf("    \033[32msendmsg <PID> <m>\033[0m  Send message to terminal\n");
    printf("    \033[32mbroadcast <msg>\033[0m    Message all terminals\n");
    printf("    \033[32mcheckmsg\033[0m           Check messages\n");
    printf("    \033[32mreadmsg\033[0m            Read messages\n");
    printf("    \033[32mclearmsg\033[0m           Clear inbox\n\n");
    
    printf("\033[33m  Expression & Math:\033[0m\n");
    printf("    \033[32mcalc <expr>\033[0m       Calculator (+,-,*,/,^,%%,parens)\n");
    printf("    \033[32minfix2postfix\033[0m     Convert to postfix\n");
    printf("    \033[32minfix2prefix\033[0m      Convert to prefix\n");
    printf("    \033[32mvisualeval\033[0m        Step-by-step evaluation\n\n");
    
    printf("\033[33m  Encoding & Conversion:\033[0m\n");
    printf("    \033[32mbase64 e|d <t>\033[0m    Base64 encode/decode\n");
    printf("    \033[32mmorse e|d <t>\033[0m     Morse code\n");
    printf("    \033[32mrot13 <text>\033[0m      ROT13 cipher\n");
    printf("    \033[32mbinary <num>\033[0m      Decimal to binary\n");
    printf("    \033[32mhex <num>\033[0m         Decimal to hex\n");
    printf("    \033[32moctal <num>\033[0m       Decimal to octal\n\n");
    
    printf("\033[33m  Productivity:\033[0m\n");
    printf("    \033[32mquicknote add\033[0m     Add quick note\n");
    printf("    \033[32mquicknote list\033[0m    List notes\n");
    printf("    \033[32mrecent\033[0m            Recently modified files\n\n");
    
    printf("\033[33m  Automation:\033[0m\n");
    printf("    \033[32mmacro define <n>\033[0m  Record commands\n");
    printf("    \033[32mmacro end\033[0m         Stop recording\n");
    printf("    \033[32mmacro run <n>\033[0m     Run macro\n");
    printf("    \033[32mteach on|off\033[0m      Teaching mode\n\n");
    
    printf("\033[33m  Natural Language:\033[0m\n");
    printf("    \"show files\"        → ls -la\n");
    printf("    \"go home\"           → cd ~\n");
    printf("    \"find large files\"  → find . -size +1M\n");
    printf("    \"calculate 2+3*4\"   → calc 2+3*4\n\n");
    
    printf("\033[32m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
}
