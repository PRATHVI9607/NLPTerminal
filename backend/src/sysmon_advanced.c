/**
 * System Monitor - Linux Only
 * Shows CPU, Memory, Disk usage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include "sysmon_advanced.h"

// Print progress bar (plain text, no ANSI)
static void print_bar(double percent, int width) {
    int filled = (int)(percent * width / 100.0);
    if (filled > width) filled = width;
    if (filled < 0) filled = 0;
    
    printf("[");
    for (int i = 0; i < width; i++) {
        printf(i < filled ? "#" : "-");
    }
    printf("] %5.1f%%", percent);
}

static void format_bytes(uint64_t bytes, char *out, size_t size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double val = (double)bytes;
    while (val >= 1024 && unit < 4) { val /= 1024; unit++; }
    snprintf(out, size, "%.1f %s", val, units[unit]);
}

// CPU Info
CPUInfo sysmon_get_cpu_info(void) {
    CPUInfo info = {0};
    
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "processor", 9) == 0) info.num_cores++;
            else if (strncmp(line, "model name", 10) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    colon += 2;
                    char *nl = strchr(colon, '\n');
                    if (nl) *nl = '\0';
                    strncpy(info.model_name, colon, sizeof(info.model_name) - 1);
                }
            }
        }
        fclose(fp);
    }
    
    // CPU usage from /proc/stat
    fp = fopen("/proc/stat", "r");
    if (fp) {
        unsigned long user, nice, system, idle;
        if (fscanf(fp, "cpu %lu %lu %lu %lu", &user, &nice, &system, &idle) == 4) {
            unsigned long total = user + nice + system + idle;
            unsigned long used = user + nice + system;
            if (total > 0) info.usage_percent = (double)used / total * 100.0;
        }
        fclose(fp);
    }
    return info;
}

// Memory Info
MemoryInfo sysmon_get_memory_info(void) {
    MemoryInfo info = {0};
    
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp) {
        char line[256];
        unsigned long mem_total = 0, mem_available = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "MemTotal:", 9) == 0)
                sscanf(line, "MemTotal: %lu", &mem_total);
            else if (strncmp(line, "MemAvailable:", 13) == 0)
                sscanf(line, "MemAvailable: %lu", &mem_available);
        }
        fclose(fp);
        
        info.total_bytes = mem_total * 1024ULL;
        info.free_bytes = mem_available * 1024ULL;
        info.used_bytes = info.total_bytes - info.free_bytes;
        if (info.total_bytes > 0)
            info.usage_percent = (double)info.used_bytes / info.total_bytes * 100.0;
    }
    return info;
}

// Disk Info
int sysmon_get_disk_info(DiskInfo *disks, int max_disks) {
    int count = 0;
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp) && count < max_disks) {
            char device[128], mount[128], fs[32];
            if (sscanf(line, "%127s %127s %31s", device, mount, fs) == 3) {
                if (strncmp(device, "/dev/", 5) != 0) continue;
                if (strcmp(fs, "squashfs") == 0) continue;
                
                struct statvfs st;
                if (statvfs(mount, &st) == 0 && st.f_blocks > 0) {
                    DiskInfo *d = &disks[count];
                    strncpy(d->mount_point, mount, sizeof(d->mount_point) - 1);
                    strncpy(d->filesystem, fs, sizeof(d->filesystem) - 1);
                    d->total_bytes = (uint64_t)st.f_blocks * st.f_frsize;
                    d->free_bytes = (uint64_t)st.f_bfree * st.f_frsize;
                    d->used_bytes = d->total_bytes - d->free_bytes;
                    d->usage_percent = (double)d->used_bytes / d->total_bytes * 100.0;
                    count++;
                }
            }
        }
        fclose(fp);
    }
    return count;
}

// Process Info
int sysmon_get_process_info(ProcessInfo *procs, int max_procs) {
    int count = 0;
    DIR *dir = opendir("/proc");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) && count < max_procs) {
            int pid = atoi(entry->d_name);
            if (pid <= 0) continue;
            
            char path[128];
            snprintf(path, sizeof(path), "/proc/%d/comm", pid);
            
            FILE *fp = fopen(path, "r");
            if (fp) {
                ProcessInfo *p = &procs[count];
                p->pid = pid;
                if (fgets(p->name, sizeof(p->name), fp)) {
                    char *nl = strchr(p->name, '\n');
                    if (nl) *nl = '\0';
                }
                fclose(fp);
                
                snprintf(path, sizeof(path), "/proc/%d/statm", pid);
                fp = fopen(path, "r");
                if (fp) {
                    unsigned long rss;
                    if (fscanf(fp, "%*u %lu", &rss) == 1)
                        p->memory_bytes = rss * 4096;
                    fclose(fp);
                }
                count++;
            }
        }
        closedir(dir);
    }
    return count;
}

// Uptime
UptimeInfo sysmon_get_uptime(void) {
    UptimeInfo info = {0};
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp) {
        double uptime;
        if (fscanf(fp, "%lf", &uptime) == 1)
            info.total_seconds = (uint64_t)uptime;
        fclose(fp);
    }
    info.days = info.total_seconds / 86400;
    info.hours = (info.total_seconds % 86400) / 3600;
    info.minutes = (info.total_seconds % 3600) / 60;
    info.seconds = info.total_seconds % 60;
    return info;
}

// Network Info
int sysmon_get_network_info(NetworkInfo *nets, int max_nets) {
    int count = 0;
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp) {
        char line[256];
        fgets(line, sizeof(line), fp);
        fgets(line, sizeof(line), fp);
        
        while (fgets(line, sizeof(line), fp) && count < max_nets) {
            NetworkInfo *n = &nets[count];
            unsigned long rx, tx;
            char name[32];
            if (sscanf(line, " %31[^:]: %lu %*u %*u %*u %*u %*u %*u %*u %lu", name, &rx, &tx) == 3) {
                strncpy(n->name, name, sizeof(n->name) - 1);
                n->bytes_recv = rx;
                n->bytes_sent = tx;
                n->is_up = 1;
                count++;
            }
        }
        fclose(fp);
    }
    return count;
}

// Display compact
void sysmon_display_compact(void) {
    CPUInfo cpu = sysmon_get_cpu_info();
    MemoryInfo mem = sysmon_get_memory_info();
    
    char used[32], total[32];
    format_bytes(mem.used_bytes, used, sizeof(used));
    format_bytes(mem.total_bytes, total, sizeof(total));
    
    printf("\n========== SYSTEM STATUS ==========\n\n");
    printf("  [CPU] ");
    print_bar(cpu.usage_percent, 30);
    printf("  (%d cores)\n", cpu.num_cores);
    
    printf("  [RAM] ");
    print_bar(mem.usage_percent, 30);
    printf("  (%s / %s)\n\n", used, total);
}

// Display full
void sysmon_display_full(void) {
    // Header
    printf("\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|            [*] SYSTEM RESOURCE MONITOR [*]                   |\n");
    printf("+--------------------------------------------------------------+\n\n");
    
    UptimeInfo uptime = sysmon_get_uptime();
    printf("[i] Uptime: %d days, %02d:%02d:%02d\n\n", 
           uptime.days, uptime.hours, uptime.minutes, uptime.seconds);
    
    // CPU Section
    CPUInfo cpu = sysmon_get_cpu_info();
    printf("+--------------------------------------------------------------+\n");
    printf("|  [CPU] PROCESSOR INFORMATION                                 |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|  Model: %-50.50s  |\n", 
           cpu.model_name[0] ? cpu.model_name : "Unknown");
    printf("|  Cores: %-3d                                                  |\n", cpu.num_cores);
    printf("|  Usage: "); 
    print_bar(cpu.usage_percent, 40); 
    printf("       |\n");
    printf("+--------------------------------------------------------------+\n\n");
    
    // Memory Section
    MemoryInfo mem = sysmon_get_memory_info();
    char used[32], free_s[32], total[32];
    format_bytes(mem.used_bytes, used, sizeof(used));
    format_bytes(mem.free_bytes, free_s, sizeof(free_s));
    format_bytes(mem.total_bytes, total, sizeof(total));
    
    printf("+--------------------------------------------------------------+\n");
    printf("|  [RAM] MEMORY                                                |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|  Total: %-10s  Used: %-10s  Free: %-10s  |\n", total, used, free_s);
    printf("|  Usage: "); 
    print_bar(mem.usage_percent, 40); 
    printf("       |\n");
    printf("+--------------------------------------------------------------+\n\n");
    
    // Disk Section
    DiskInfo disks[10];
    int disk_count = sysmon_get_disk_info(disks, 10);
    printf("+--------------------------------------------------------------+\n");
    printf("|  [HDD] DISK STORAGE                                          |\n");
    printf("+--------------------------------------------------------------+\n");
    for (int i = 0; i < disk_count && i < 4; i++) {
        char d_used[32], d_total[32];
        format_bytes(disks[i].used_bytes, d_used, sizeof(d_used));
        format_bytes(disks[i].total_bytes, d_total, sizeof(d_total));
        printf("|  %-12.12s ", disks[i].mount_point);
        print_bar(disks[i].usage_percent, 20);
        printf(" %s/%s\n", d_used, d_total);
    }
    printf("+--------------------------------------------------------------+\n\n");
    
    // Processes Section
    printf("+--------------------------------------------------------------+\n");
    printf("|  [PS] TOP PROCESSES                                          |\n");
    printf("+--------------------------------------------------------------+\n");
    printf("|  %-8s %-30s %s               |\n", "PID", "NAME", "MEMORY");
    printf("|  -------- ------------------------------ ----------          |\n");
    ProcessInfo procs[50];
    int pc = sysmon_get_process_info(procs, 50);
    int shown = 0;
    for (int i = 0; i < pc && shown < 6; i++) {
        if (procs[i].memory_bytes > 1024*1024) {
            char m[32]; format_bytes(procs[i].memory_bytes, m, sizeof(m));
            printf("|  %-8d %-30.30s %-10s          |\n", 
                   procs[i].pid, procs[i].name, m);
            shown++;
        }
    }
    printf("+--------------------------------------------------------------+\n\n");
}

// Live display
void sysmon_display_live(int duration_sec) {
    printf("=== LIVE MONITOR ===\n\n");
    for (int t = 0; t < duration_sec; t++) {
        CPUInfo cpu = sysmon_get_cpu_info();
        printf("CPU: "); print_bar(cpu.usage_percent, 50); printf("\n");
        
        MemoryInfo mem = sysmon_get_memory_info();
        char u[32], t2[32];
        format_bytes(mem.used_bytes, u, sizeof(u));
        format_bytes(mem.total_bytes, t2, sizeof(t2));
        printf("MEM: "); print_bar(mem.usage_percent, 50);
        printf(" (%s/%s)\n", u, t2);
        
        printf("Refresh: %d/%d\n\n", t+1, duration_sec);
        fflush(stdout);
        sleep(1);
    }
}
