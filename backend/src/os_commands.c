/**
 * OS Commands - System Monitoring using /proc filesystem
 * All commands read directly from kernel interfaces
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <pwd.h>
#include <utmp.h>
#include <time.h>
#include <ctype.h>
#include "os_commands.h"

#define BUFSIZE 4096

// Helper: format bytes
static void format_bytes(unsigned long long bytes, char *out, size_t sz) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double size = bytes;
    while (size >= 1024 && i < 4) { size /= 1024; i++; }
    snprintf(out, sz, "%.1f %s", size, units[i]);
}

// Helper: read a value from /proc file
static long long read_proc_value(const char *file, const char *key) {
    FILE *fp = fopen(file, "r");
    if (!fp) return -1;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, key)) {
            char *p = line;
            while (*p && !isdigit(*p)) p++;
            long long val = atoll(p);
            fclose(fp);
            return val;
        }
    }
    fclose(fp);
    return -1;
}

// cpuinfo - CPU information and usage
void do_cpuinfo(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== CPU INFORMATION ===\033[0m\n\n");
    
    // Model name
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        int cores = 0;
        char model[256] = "";
        double mhz = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "model name", 10) == 0 && model[0] == 0) {
                char *p = strchr(line, ':');
                if (p) strncpy(model, p + 2, sizeof(model) - 1);
                model[strcspn(model, "\n")] = 0;
            }
            if (strncmp(line, "processor", 9) == 0) cores++;
            if (strncmp(line, "cpu MHz", 7) == 0 && mhz == 0) {
                char *p = strchr(line, ':');
                if (p) mhz = atof(p + 2);
            }
        }
        fclose(fp);
        
        printf("Model:      %s\n", model);
        printf("Cores:      %d\n", cores);
        printf("Frequency:  %.0f MHz\n", mhz);
    }
    
    // CPU usage from /proc/stat
    fp = fopen("/proc/stat", "r");
    if (fp) {
        char line[256];
        if (fgets(line, sizeof(line), fp)) {
            unsigned long long user, nice, system, idle, iowait, irq, softirq;
            if (sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu",
                       &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7) {
                unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
                unsigned long long busy = user + nice + system + irq + softirq;
                double usage = (double)busy / total * 100;
                printf("Usage:      %.1f%%\n", usage);
                
                // Bar
                int bars = (int)(usage / 5);
                printf("            [");
                for (int i = 0; i < 20; i++) printf("%s", i < bars ? "#" : "-");
                printf("] %.1f%%\n", usage);
            }
        }
        fclose(fp);
    }
    
    // Per-core usage
    printf("\nPer-core usage:\n");
    fp = fopen("/proc/stat", "r");
    if (fp) {
        char line[256];
        fgets(line, sizeof(line), fp);  // Skip first line (total)
        int core = 0;
        while (fgets(line, sizeof(line), fp) && strncmp(line, "cpu", 3) == 0) {
            unsigned long long user, nice, system, idle, iowait, irq, softirq;
            if (sscanf(line, "cpu%*d %llu %llu %llu %llu %llu %llu %llu",
                       &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7) {
                unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
                unsigned long long busy = user + nice + system + irq + softirq;
                double usage = (double)busy / total * 100;
                int bars = (int)(usage / 5);
                printf("  Core %d: [", core);
                for (int i = 0; i < 20; i++) printf("%s", i < bars ? "#" : "-");
                printf("] %5.1f%%\n", usage);
                core++;
            }
        }
        fclose(fp);
    }
    printf("\n");
}

// meminfo - Memory usage details
void do_meminfo(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== MEMORY INFORMATION ===\033[0m\n\n");
    
    long long total = read_proc_value("/proc/meminfo", "MemTotal:");
    long long free = read_proc_value("/proc/meminfo", "MemFree:");
    long long avail = read_proc_value("/proc/meminfo", "MemAvailable:");
    long long buffers = read_proc_value("/proc/meminfo", "Buffers:");
    long long cached = read_proc_value("/proc/meminfo", "Cached:");
    
    char ts[32], fs[32], as[32], bs[32], cs[32];
    format_bytes(total * 1024, ts, 32);
    format_bytes(free * 1024, fs, 32);
    format_bytes(avail * 1024, as, 32);
    format_bytes(buffers * 1024, bs, 32);
    format_bytes(cached * 1024, cs, 32);
    
    long long used = total - avail;
    double pct = (double)used / total * 100;
    
    printf("Total:      %s\n", ts);
    printf("Used:       %s (%.1f%%)\n", ts, pct);
    format_bytes(used * 1024, ts, 32);
    printf("            %s actual\n", ts);
    printf("Available:  %s\n", as);
    printf("Free:       %s\n", fs);
    printf("Buffers:    %s\n", bs);
    printf("Cached:     %s\n", cs);
    
    int bars = (int)(pct / 5);
    printf("\n[");
    for (int i = 0; i < 20; i++) printf("%s", i < bars ? "#" : "-");
    printf("] %.1f%% used\n\n", pct);
}

// swapinfo - Swap memory info
void do_swapinfo(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== SWAP INFORMATION ===\033[0m\n\n");
    
    long long total = read_proc_value("/proc/meminfo", "SwapTotal:");
    long long free = read_proc_value("/proc/meminfo", "SwapFree:");
    long long cached = read_proc_value("/proc/meminfo", "SwapCached:");
    
    if (total <= 0) {
        printf("No swap configured\n\n");
        return;
    }
    
    char ts[32], fs[32], us[32], cs[32];
    long long used = total - free;
    format_bytes(total * 1024, ts, 32);
    format_bytes(free * 1024, fs, 32);
    format_bytes(used * 1024, us, 32);
    format_bytes(cached * 1024, cs, 32);
    
    double pct = (double)used / total * 100;
    
    printf("Total:   %s\n", ts);
    printf("Used:    %s (%.1f%%)\n", us, pct);
    printf("Free:    %s\n", fs);
    printf("Cached:  %s\n", cs);
    
    // Show swap devices
    printf("\nSwap devices:\n");
    FILE *fp = fopen("/proc/swaps", "r");
    if (fp) {
        char line[256];
        fgets(line, sizeof(line), fp);  // header
        while (fgets(line, sizeof(line), fp)) {
            printf("  %s", line);
        }
        fclose(fp);
    }
    printf("\n");
}

// diskinfo - Disk partitions and usage
void do_diskinfo(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== DISK INFORMATION ===\033[0m\n\n");
    printf("%-20s %10s %10s %10s %6s\n", "Mount", "Total", "Used", "Free", "Use%");
    printf("%-20s %10s %10s %10s %6s\n", "-----", "-----", "----", "----", "----");
    
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) return;
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char dev[128], mnt[128], fs[32];
        if (sscanf(line, "%s %s %s", dev, mnt, fs) == 3) {
            // Only real filesystems
            if (strncmp(dev, "/dev/", 5) == 0 || strcmp(fs, "tmpfs") == 0) {
                struct statvfs st;
                if (statvfs(mnt, &st) == 0 && st.f_blocks > 0) {
                    unsigned long long total = (unsigned long long)st.f_blocks * st.f_frsize;
                    unsigned long long free = (unsigned long long)st.f_bfree * st.f_frsize;
                    unsigned long long used = total - free;
                    
                    char ts[16], us[16], fs2[16];
                    format_bytes(total, ts, 16);
                    format_bytes(used, us, 16);
                    format_bytes(free, fs2, 16);
                    
                    printf("%-20s %10s %10s %10s %5.1f%%\n", 
                           mnt, ts, us, fs2, 100.0 * used / total);
                }
            }
        }
    }
    fclose(fp);
    printf("\n");
}

// proclist - List running processes
void do_proclist(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== PROCESS LIST ===\033[0m\n\n");
    printf("%-8s %-10s %-8s %-8s %-30s\n", "PID", "USER", "STATE", "MEM(KB)", "COMMAND");
    printf("%-8s %-10s %-8s %-8s %-30s\n", "---", "----", "-----", "-------", "-------");
    
    DIR *d = opendir("/proc");
    if (!d) return;
    
    struct dirent *ent;
    int count = 0;
    while ((ent = readdir(d)) && count < 50) {
        int pid = atoi(ent->d_name);
        if (pid > 0) {
            char path[128], buf[512];
            
            // Get comm
            char comm[64] = "";
            snprintf(path, sizeof(path), "/proc/%d/comm", pid);
            FILE *fp = fopen(path, "r");
            if (fp) { fgets(comm, sizeof(comm), fp); fclose(fp); }
            comm[strcspn(comm, "\n")] = 0;
            
            // Get status
            char state = '?';
            int uid = -1;
            long vmrss = 0;
            snprintf(path, sizeof(path), "/proc/%d/status", pid);
            fp = fopen(path, "r");
            if (fp) {
                while (fgets(buf, sizeof(buf), fp)) {
                    if (strncmp(buf, "State:", 6) == 0) {
                        char *p = buf + 6;
                        while (*p == ' ' || *p == '\t') p++;
                        state = *p;
                    }
                    if (strncmp(buf, "Uid:", 4) == 0) {
                        sscanf(buf + 4, "%d", &uid);
                    }
                    if (strncmp(buf, "VmRSS:", 6) == 0) {
                        sscanf(buf + 6, "%ld", &vmrss);
                    }
                }
                fclose(fp);
            }
            
            // Get username
            char user[16] = "?";
            struct passwd *pw = getpwuid(uid);
            if (pw) strncpy(user, pw->pw_name, 15);
            
            printf("%-8d %-10s %-8c %-8ld %-30s\n", pid, user, state, vmrss, comm);
            count++;
        }
    }
    closedir(d);
    printf("\n[Showing first 50 processes]\n\n");
}

// proctop - Top N processes by memory
void do_proctop(char **args) {
    int n = args[1] ? atoi(args[1]) : 10;
    if (n <= 0) n = 10;
    
    printf("\n\033[1;36m=== TOP %d PROCESSES BY MEMORY ===\033[0m\n\n", n);
    printf("%-8s %-8s %-30s\n", "PID", "MEM(KB)", "COMMAND");
    printf("%-8s %-8s %-30s\n", "---", "-------", "-------");
    
    struct { int pid; long mem; char comm[64]; } procs[500];
    int count = 0;
    
    DIR *d = opendir("/proc");
    if (!d) return;
    
    struct dirent *ent;
    while ((ent = readdir(d)) && count < 500) {
        int pid = atoi(ent->d_name);
        if (pid > 0) {
            char path[128], buf[256];
            
            snprintf(path, sizeof(path), "/proc/%d/comm", pid);
            FILE *fp = fopen(path, "r");
            if (fp) {
                fgets(procs[count].comm, sizeof(procs[count].comm), fp);
                procs[count].comm[strcspn(procs[count].comm, "\n")] = 0;
                fclose(fp);
            }
            
            snprintf(path, sizeof(path), "/proc/%d/status", pid);
            fp = fopen(path, "r");
            procs[count].mem = 0;
            if (fp) {
                while (fgets(buf, sizeof(buf), fp)) {
                    if (strncmp(buf, "VmRSS:", 6) == 0) {
                        sscanf(buf + 6, "%ld", &procs[count].mem);
                        break;
                    }
                }
                fclose(fp);
            }
            procs[count].pid = pid;
            count++;
        }
    }
    closedir(d);
    
    // Sort by memory (bubble sort for simplicity)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (procs[j].mem < procs[j+1].mem) {
                typeof(procs[0]) tmp = procs[j];
                procs[j] = procs[j+1];
                procs[j+1] = tmp;
            }
        }
    }
    
    for (int i = 0; i < n && i < count; i++) {
        printf("%-8d %-8ld %-30s\n", procs[i].pid, procs[i].mem, procs[i].comm);
    }
    printf("\n");
}

// prockill - Kill a process
void do_prockill(char **args) {
    if (!args[1]) {
        fprintf(stderr, "Usage: prockill <pid>\n");
        return;
    }
    int pid = atoi(args[1]);
    if (pid <= 0) {
        fprintf(stderr, "Invalid PID: %s\n", args[1]);
        return;
    }
    if (kill(pid, SIGTERM) == 0) {
        printf("\033[1;32mProcess %d terminated\033[0m\n", pid);
    } else {
        perror("prockill");
    }
}

// netinfo - Network interfaces
void do_netinfo(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== NETWORK INTERFACES ===\033[0m\n\n");
    
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) return;
    
    printf("%-12s %15s %15s %10s %10s\n", "Interface", "RX Bytes", "TX Bytes", "RX Pkts", "TX Pkts");
    printf("%-12s %15s %15s %10s %10s\n", "---------", "--------", "--------", "-------", "-------");
    
    char line[256];
    fgets(line, sizeof(line), fp);  // header
    fgets(line, sizeof(line), fp);  // header
    
    while (fgets(line, sizeof(line), fp)) {
        char iface[32];
        unsigned long long rx_bytes, rx_packets, tx_bytes, tx_packets;
        unsigned long long dummy;
        
        char *p = line;
        while (*p == ' ') p++;
        char *colon = strchr(p, ':');
        if (colon) {
            *colon = 0;
            memset(iface, 0, 32);
            memcpy(iface, p, strlen(p) < 31 ? strlen(p) : 31);
            
            if (sscanf(colon + 1, "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                       &rx_bytes, &rx_packets, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy,
                       &tx_bytes, &tx_packets) >= 4) {
                char rxs[16], txs[16];
                format_bytes(rx_bytes, rxs, 16);
                format_bytes(tx_bytes, txs, 16);
                printf("%-12s %15s %15s %10llu %10llu\n", iface, rxs, txs, rx_packets, tx_packets);
            }
        }
    }
    fclose(fp);
    printf("\n");
}

// netstat - Network statistics
void do_netstat(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== NETWORK STATISTICS ===\033[0m\n\n");
    
    // From /proc/net/snmp
    FILE *fp = fopen("/proc/net/snmp", "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "Tcp:", 4) == 0) {
                // Get next line with values
                if (fgets(line, sizeof(line), fp)) {
                    printf("TCP: %s", line);
                }
            }
            if (strncmp(line, "Udp:", 4) == 0) {
                if (fgets(line, sizeof(line), fp)) {
                    printf("UDP: %s", line);
                }
            }
        }
        fclose(fp);
    }
    printf("\n");
}

// connections - Active connections
void do_connections(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== ACTIVE CONNECTIONS ===\033[0m\n\n");
    printf("%-6s %-25s %-25s %-12s\n", "Proto", "Local Address", "Remote Address", "State");
    printf("%-6s %-25s %-25s %-12s\n", "-----", "-------------", "--------------", "-----");
    
    // TCP connections from /proc/net/tcp
    FILE *fp = fopen("/proc/net/tcp", "r");
    if (fp) {
        char line[256];
        fgets(line, sizeof(line), fp);  // header
        
        while (fgets(line, sizeof(line), fp)) {
            unsigned int local_ip, local_port, remote_ip, remote_port, state;
            if (sscanf(line, "%*d: %x:%x %x:%x %x",
                       &local_ip, &local_port, &remote_ip, &remote_port, &state) == 5) {
                
                const char *state_str;
                switch (state) {
                    case 1: state_str = "ESTABLISHED"; break;
                    case 2: state_str = "SYN_SENT"; break;
                    case 6: state_str = "TIME_WAIT"; break;
                    case 7: state_str = "CLOSE"; break;
                    case 10: state_str = "LISTEN"; break;
                    default: state_str = "OTHER"; break;
                }
                
                char local[32], remote[32];
                snprintf(local, sizeof(local), "%d.%d.%d.%d:%d",
                        local_ip & 0xff, (local_ip >> 8) & 0xff,
                        (local_ip >> 16) & 0xff, (local_ip >> 24) & 0xff, local_port);
                snprintf(remote, sizeof(remote), "%d.%d.%d.%d:%d",
                        remote_ip & 0xff, (remote_ip >> 8) & 0xff,
                        (remote_ip >> 16) & 0xff, (remote_ip >> 24) & 0xff, remote_port);
                
                printf("%-6s %-25s %-25s %-12s\n", "tcp", local, remote, state_str);
            }
        }
        fclose(fp);
    }
    printf("\n");
}

// paging - Paging statistics from /proc/vmstat
void do_paging(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== PAGING STATISTICS ===\033[0m\n");
    printf("Source: /proc/vmstat\n\n");
    
    FILE *fp = fopen("/proc/vmstat", "r");
    if (!fp) {
        perror("Cannot read /proc/vmstat");
        return;
    }
    
    printf("%-20s %15s\n", "Statistic", "Count");
    printf("%-20s %15s\n", "---------", "-----");
    
    char line[128];
    const char *keys[] = {"pgpgin", "pgpgout", "pswpin", "pswpout", "pgfault", "pgmajfault",
                          "pgfree", "pgactivate", "pgdeactivate", "pglazyfree",
                          "pglazyfreed", "pgsteal_kswapd", "pgsteal_direct", NULL};
    
    while (fgets(line, sizeof(line), fp)) {
        char key[64];
        long long val;
        if (sscanf(line, "%s %lld", key, &val) == 2) {
            for (int i = 0; keys[i]; i++) {
                if (strcmp(key, keys[i]) == 0) {
                    printf("%-20s %15lld\n", key, val);
                    break;
                }
            }
        }
    }
    fclose(fp);
    
    printf("\n\033[2mKey metrics:\033[0m\n");
    printf("  pgpgin/pgpgout - Pages read/written to disk\n");
    printf("  pswpin/pswpout - Swap pages in/out\n");
    printf("  pgfault        - Total page faults\n");
    printf("  pgmajfault     - Major page faults (required I/O)\n\n");
}

// vmstat - Full virtual memory statistics
void do_vmstat(char **args) {
    int lines = args[1] ? atoi(args[1]) : 30;
    
    printf("\n\033[1;36m=== VMSTAT ===\033[0m\n\n");
    
    FILE *fp = fopen("/proc/vmstat", "r");
    if (!fp) return;
    
    char line[128];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < lines) {
        printf("%s", line);
        count++;
    }
    fclose(fp);
    
    if (count >= lines) {
        printf("...[truncated, use 'vmstat N' for more lines]\n");
    }
    printf("\n");
}

// zoneinfo - Memory zone information
void do_zoneinfo(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== MEMORY ZONES ===\033[0m\n\n");
    
    FILE *fp = fopen("/proc/zoneinfo", "r");
    if (!fp) return;
    
    char line[256];
    int in_zone = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Node", 4) == 0) {
            printf("\n\033[1m%s\033[0m", line);
            in_zone = 1;
        } else if (in_zone && (strstr(line, "free") || strstr(line, "min") || 
                               strstr(line, "low") || strstr(line, "high") ||
                               strstr(line, "present") || strstr(line, "managed"))) {
            printf("%s", line);
        }
    }
    fclose(fp);
    printf("\n");
}

// loadavg - Load averages
void do_loadavg(char **args) {
    (void)args;
    
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) return;
    
    char line[128];
    if (fgets(line, sizeof(line), fp)) {
        double l1, l5, l15;
        int running, total;
        if (sscanf(line, "%lf %lf %lf %d/%d", &l1, &l5, &l15, &running, &total) == 5) {
            printf("\n\033[1;36m=== LOAD AVERAGE ===\033[0m\n\n");
            printf("1 min:   %.2f\n", l1);
            printf("5 min:   %.2f\n", l5);
            printf("15 min:  %.2f\n", l15);
            printf("\nRunning: %d / %d processes\n\n", running, total);
        }
    }
    fclose(fp);
}

// kernelinfo - Kernel version
void do_kernelinfo(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== KERNEL INFO ===\033[0m\n\n");
    
    FILE *fp = fopen("/proc/version", "r");
    if (fp) {
        char line[512];
        if (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }
        fclose(fp);
    }
    
    fp = fopen("/proc/sys/kernel/hostname", "r");
    if (fp) {
        char line[128];
        if (fgets(line, sizeof(line), fp)) {
            printf("Hostname: %s", line);
        }
        fclose(fp);
    }
    printf("\n");
}

// filesystems - Supported filesystems
void do_filesystems(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== SUPPORTED FILESYSTEMS ===\033[0m\n\n");
    
    FILE *fp = fopen("/proc/filesystems", "r");
    if (fp) {
        char line[128];
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }
        fclose(fp);
    }
    printf("\n");
}

// mounts - Mounted filesystems
void do_mounts(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== MOUNTED FILESYSTEMS ===\033[0m\n\n");
    printf("%-30s %-25s %-10s\n", "Device", "Mount Point", "Type");
    printf("%-30s %-25s %-10s\n", "------", "-----------", "----");
    
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            char dev[128], mnt[128], type[32];
            if (sscanf(line, "%s %s %s", dev, mnt, type) == 3) {
                printf("%-30s %-25s %-10s\n", dev, mnt, type);
            }
        }
        fclose(fp);
    }
    printf("\n");
}

// modules - Loaded kernel modules
void do_modules(char **args) {
    int n = args[1] ? atoi(args[1]) : 30;
    
    printf("\n\033[1;36m=== KERNEL MODULES ===\033[0m\n\n");
    printf("%-25s %10s %6s\n", "Module", "Size", "Used");
    printf("%-25s %10s %6s\n", "------", "----", "----");
    
    FILE *fp = fopen("/proc/modules", "r");
    if (fp) {
        char line[256];
        int count = 0;
        while (fgets(line, sizeof(line), fp) && count < n) {
            char name[64];
            long size;
            int used;
            if (sscanf(line, "%s %ld %d", name, &size, &used) >= 2) {
                char sz[16];
                format_bytes(size, sz, 16);
                printf("%-25s %10s %6d\n", name, sz, used);
                count++;
            }
        }
        fclose(fp);
    }
    printf("\n");
}

// battery - Battery status
void do_battery(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== BATTERY STATUS ===\033[0m\n\n");
    
    // Try /sys/class/power_supply/BAT0
    FILE *fp = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (fp) {
        int capacity;
        if (fscanf(fp, "%d", &capacity) == 1) {
            printf("Capacity: %d%%\n", capacity);
        }
        fclose(fp);
        
        fp = fopen("/sys/class/power_supply/BAT0/status", "r");
        if (fp) {
            char status[32];
            if (fgets(status, sizeof(status), fp)) {
                status[strcspn(status, "\n")] = 0;
                printf("Status:   %s\n", status);
            }
            fclose(fp);
        }
    } else {
        printf("No battery detected or /sys/class/power_supply/BAT0 not available\n");
    }
    printf("\n");
}

// sensors - Temperature sensors
void do_sensors(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== TEMPERATURE SENSORS ===\033[0m\n\n");
    
    // Try thermal zones
    for (int i = 0; i < 10; i++) {
        char path[64], type[64] = "";
        snprintf(path, sizeof(path), "/sys/class/thermal/thermal_zone%d/type", i);
        
        FILE *fp = fopen(path, "r");
        if (!fp) break;
        fgets(type, sizeof(type), fp);
        type[strcspn(type, "\n")] = 0;
        fclose(fp);
        
        snprintf(path, sizeof(path), "/sys/class/thermal/thermal_zone%d/temp", i);
        fp = fopen(path, "r");
        if (fp) {
            int temp;
            if (fscanf(fp, "%d", &temp) == 1) {
                printf("%-20s: %.1f C\n", type, temp / 1000.0);
            }
            fclose(fp);
        }
    }
    printf("\n");
}

// interrupts - Interrupt statistics
void do_interrupts(char **args) {
    int n = args[1] ? atoi(args[1]) : 15;
    
    printf("\n\033[1;36m=== INTERRUPTS ===\033[0m\n\n");
    
    FILE *fp = fopen("/proc/interrupts", "r");
    if (fp) {
        char line[512];
        int count = 0;
        while (fgets(line, sizeof(line), fp) && count < n) {
            printf("%s", line);
            count++;
        }
        fclose(fp);
    }
    printf("...[use 'interrupts N' for more]\n\n");
}

// users - Logged in users
void do_users(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== LOGGED IN USERS ===\033[0m\n\n");
    printf("%-12s %-12s %-20s\n", "User", "Terminal", "Login Time");
    printf("%-12s %-12s %-20s\n", "----", "--------", "----------");
    
    setutent();
    struct utmp *entry;
    while ((entry = getutent())) {
        if (entry->ut_type == USER_PROCESS) {
            time_t t = entry->ut_tv.tv_sec;
            char *ts = ctime(&t);
            if (ts) ts[strcspn(ts, "\n")] = 0;
            printf("%-12s %-12s %-20s\n", entry->ut_user, entry->ut_line, ts ? ts : "");
        }
    }
    endutent();
    printf("\n");
}

// envvar - Environment variables
void do_envvar(char **args) {
    if (args[1]) {
        char *val = getenv(args[1]);
        if (val) {
            printf("%s=%s\n", args[1], val);
        } else {
            printf("%s: not set\n", args[1]);
        }
        return;
    }
    
    printf("\n\033[1;36m=== ENVIRONMENT VARIABLES ===\033[0m\n\n");
    
    extern char **environ;
    int count = 0;
    for (char **env = environ; *env && count < 30; env++, count++) {
        char buf[80];
        strncpy(buf, *env, 79);
        buf[79] = 0;
        printf("%s%s\n", buf, strlen(*env) > 79 ? "..." : "");
    }
    printf("\n[Showing first 30, use 'envvar NAME' for specific variable]\n\n");
}

// openfiles - Open file descriptors
void do_openfiles(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== OPEN FILES (this shell) ===\033[0m\n\n");
    
    char path[128];
    snprintf(path, sizeof(path), "/proc/%d/fd", getpid());
    
    DIR *d = opendir(path);
    if (!d) return;
    
    printf("%-5s %-50s\n", "FD", "Target");
    printf("%-5s %-50s\n", "--", "------");
    
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (ent->d_name[0] != '.') {
            char fdpath[512], target[256];
            snprintf(fdpath, sizeof(fdpath), "%s/%s", path, ent->d_name);
            ssize_t len = readlink(fdpath, target, sizeof(target) - 1);
            if (len > 0) {
                target[len] = 0;
                printf("%-5s %-50s\n", ent->d_name, target);
            }
        }
    }
    closedir(d);
    printf("\n");
}

// sockets - Socket statistics
void do_sockets(char **args) {
    (void)args;
    
    printf("\n\033[1;36m=== SOCKET STATISTICS ===\033[0m\n\n");
    
    FILE *fp = fopen("/proc/net/sockstat", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }
        fclose(fp);
    }
    printf("\n");
}
