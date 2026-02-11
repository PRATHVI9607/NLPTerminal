/**
 * OS Commands Header - System Monitoring Commands
 * Real-time system information using /proc filesystem
 */

#ifndef OS_COMMANDS_H
#define OS_COMMANDS_H

// CPU & Memory
void do_cpuinfo(char **args);    // CPU information and usage
void do_meminfo(char **args);    // Memory usage details
void do_swapinfo(char **args);   // Swap memory info

// Disk
void do_diskinfo(char **args);   // Disk partitions and usage

// Processes
void do_proclist(char **args);   // List running processes
void do_proctop(char **args);    // Top N processes by CPU/memory
void do_prockill(char **args);   // Kill a process

// Network
void do_netinfo(char **args);    // Network interfaces
void do_netstat(char **args);    // Network statistics
void do_connections(char **args); // Active connections

// Paging & Virtual Memory
void do_paging(char **args);     // Paging statistics from /proc/vmstat
void do_vmstat(char **args);     // Virtual memory statistics
void do_zoneinfo(char **args);   // Memory zone information

// System Info
void do_loadavg(char **args);    // Load averages
void do_kernelinfo(char **args); // Kernel version
void do_filesystems(char **args);// Supported filesystems
void do_mounts(char **args);     // Mounted filesystems
void do_modules(char **args);    // Loaded kernel modules

// Hardware
void do_battery(char **args);    // Battery status
void do_sensors(char **args);    // Temperature sensors
void do_interrupts(char **args); // Interrupt statistics

// Users & Environment
void do_users(char **args);      // Logged in users
void do_envvar(char **args);     // Environment variables

// Sockets & Files
void do_openfiles(char **args);  // Open file descriptors
void do_sockets(char **args);    // Socket statistics

#endif
