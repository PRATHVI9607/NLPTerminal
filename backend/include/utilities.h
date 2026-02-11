/**
 * Advanced Utility Commands
 * Unique commands not found in standard Unix
 */

#ifndef UTILITIES_H
#define UTILITIES_H

// Encoding/Decoding
void do_base64(char **args);           // Base64 encode/decode
void do_morse(char **args);            // Morse code encode/decode
void do_rot13(char **args);            // ROT13 cipher
void do_reverse_text(char **args);     // Reverse text

// Number conversions
void do_binary(char **args);           // Decimal to binary
void do_hex(char **args);              // Decimal to hex
void do_octal(char **args);            // Decimal to octal
void do_base_convert(char **args);     // Convert between bases

// Checksums and hashing
void do_checksum(char **args);         // Multiple checksum types
void do_crc32(char **args);            // CRC32 checksum

// Compression (simple RLE)
void do_compress(char **args);         // Simple compression
void do_decompress(char **args);       // Decompress

// File comparison
void do_diff(char **args);             // Line-by-line diff

// Utilities
void do_random(char **args);           // Random number generator
void do_uuid(char **args);             // Generate UUID
void do_timer(char **args);            // Countdown timer
void do_stopwatch(char **args);        // Stopwatch
void do_color_palette(char **args);    // Show terminal colors
void do_ascii_art(char **args);        // ASCII art banner
void do_progress(char **args);         // Progress bar demo
void do_table(char **args);            // Create formatted table
void do_json_format(char **args);      // Format JSON
void do_lorem(char **args);            // Generate lorem ipsum

#endif
