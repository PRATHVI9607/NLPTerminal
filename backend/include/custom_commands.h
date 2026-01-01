/**
 * Custom Commands Header - Linux Only
 * Unique commands using direct system calls
 */

#ifndef CUSTOM_COMMANDS_H
#define CUSTOM_COMMANDS_H

// File Operations
void do_fileinfo(char **args);   // Detailed file info (size, hash, permissions)
void do_hexdump(char **args);    // Hex view of file contents
void do_duplicate(char **args);  // Find duplicate files by hash
void do_encrypt(char **args);    // Encrypt file with XOR cipher
void do_decrypt(char **args);    // Decrypt file

// File Analysis
void do_sizeof(char **args);     // Total size of matching files
void do_age(char **args);        // Find files by age
void do_freq(char **args);       // Word frequency analysis
void do_lines(char **args);      // Line/word/char statistics

// Text Processing
void do_head(char **args);       // First n lines
void do_tail(char **args);       // Last n lines
void do_wc(char **args);         // Word count
void do_grep(char **args);       // Pattern search
void do_sort(char **args);       // Sort lines
void do_uniq(char **args);       // Remove duplicate lines
void do_rev(char **args);        // Reverse lines

// Utilities
void do_quicknote(char **args);  // Quick note taking
void do_calc(char **args);       // Calculator
void do_clear(char **args);      // Clear screen

// File Compression & Conversion
void do_compress(char **args);   // Compress/decompress files (gzip)
void do_convert(char **args);    // Convert file formats

// System Info
void do_date(char **args);       // Current date/time
void do_whoami(char **args);     // Current user
void do_hostname(char **args);   // System hostname
void do_uptime(char **args);     // System uptime
void do_df(char **args);         // Disk free space
void do_ps(char **args);         // Process list
void do_kill(char **args);       // Kill process

#endif
