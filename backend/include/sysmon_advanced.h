/**
 * Advanced System Monitor Header
 * Provides detailed system information using direct system calls
 */

#ifndef SYSMON_ADVANCED_H
#define SYSMON_ADVANCED_H

#include <stdint.h>

// CPU information structure
typedef struct {
    int num_cores;
    double usage_percent;
    double per_core_usage[64];  // Up to 64 cores
    char model_name[256];
    double frequency_mhz;
} CPUInfo;

// Memory information structure
typedef struct {
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
    uint64_t cached_bytes;
    uint64_t swap_total;
    uint64_t swap_used;
    double usage_percent;
} MemoryInfo;

// Disk information structure
typedef struct {
    char mount_point[256];
    char device[256];
    char filesystem[64];
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
    double usage_percent;
} DiskInfo;

// Process information structure
typedef struct {
    int pid;
    char name[256];
    char state;
    double cpu_percent;
    uint64_t memory_bytes;
    int threads;
} ProcessInfo;

// Network interface structure
typedef struct {
    char name[64];
    uint64_t bytes_sent;
    uint64_t bytes_recv;
    uint64_t packets_sent;
    uint64_t packets_recv;
    int is_up;
} NetworkInfo;

// System uptime structure
typedef struct {
    int days;
    int hours;
    int minutes;
    int seconds;
    uint64_t total_seconds;
} UptimeInfo;

// Battery info (for laptops)
typedef struct {
    int percent;
    int is_charging;
    int is_present;
    int minutes_remaining;
} BatteryInfo;

// Get CPU information using system calls
CPUInfo sysmon_get_cpu_info(void);

// Get memory information using system calls
MemoryInfo sysmon_get_memory_info(void);

// Get disk information (returns count, fills array)
int sysmon_get_disk_info(DiskInfo *disks, int max_disks);

// Get process list (returns count, fills array)
int sysmon_get_processes(ProcessInfo *procs, int max_procs);

// Get top N processes by CPU usage
int sysmon_get_top_cpu_processes(ProcessInfo *procs, int n);

// Get top N processes by memory usage
int sysmon_get_top_memory_processes(ProcessInfo *procs, int n);

// Get network interfaces info
int sysmon_get_network_info(NetworkInfo *nets, int max_nets);

// Get system uptime
UptimeInfo sysmon_get_uptime(void);

// Get battery info
BatteryInfo sysmon_get_battery_info(void);

// Kill a process by PID using system call
int sysmon_kill_process(int pid, int signal);

// Display formatted system monitor
void sysmon_display_full(void);

// Display compact system monitor
void sysmon_display_compact(void);

// Display live updating monitor (runs for duration_sec seconds)
void sysmon_display_live(int duration_sec);

#endif
