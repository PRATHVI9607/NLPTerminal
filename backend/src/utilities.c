/**
 * Advanced Utility Commands
 * Unique and handy commands not in standard Unix
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "utilities.h"

// Base64 encoding table
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Base64 encode/decode
void do_base64(char **args) {
    if (!args[1] || !args[2]) {
        printf("Usage: base64 <encode|decode> <text|file>\n");
        printf("Example: base64 encode \"Hello World\"\n");
        return;
    }
    
    int encode = (strcmp(args[1], "encode") == 0);
    const char *input = args[2];
    
    if (encode) {
        // Simple base64 encoding
        int len = strlen(input);
        printf("Input:  %s\n", input);
        printf("Base64: ");
        
        for (int i = 0; i < len; i += 3) {
            unsigned char b1 = input[i];
            unsigned char b2 = (i + 1 < len) ? input[i + 1] : 0;
            unsigned char b3 = (i + 2 < len) ? input[i + 2] : 0;
            
            printf("%c", base64_chars[b1 >> 2]);
            printf("%c", base64_chars[((b1 & 0x03) << 4) | (b2 >> 4)]);
            printf("%c", (i + 1 < len) ? base64_chars[((b2 & 0x0F) << 2) | (b3 >> 6)] : '=');
            printf("%c", (i + 2 < len) ? base64_chars[b3 & 0x3F] : '=');
        }
        printf("\n");
    } else {
        printf("Decode: (simplified - shows concept)\n");
        printf("Base64: %s\n", input);
    }
}

// Morse code
static const char *morse_code[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",    // A-I
    ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",  // J-R
    "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."          // S-Z
};

void do_morse(char **args) {
    if (!args[1] || !args[2]) {
        printf("Usage: morse <encode|decode> <text>\n");
        printf("Example: morse encode \"SOS\"\n");
        return;
    }
    
    int encode = (strcmp(args[1], "encode") == 0);
    
    if (encode) {
        printf("Text:  ");
        for (int i = 2; args[i]; i++) {
            printf("%s ", args[i]);
        }
        printf("\nMorse: ");
        
        for (int i = 2; args[i]; i++) {
            for (int j = 0; args[i][j]; j++) {
                char c = toupper(args[i][j]);
                if (c >= 'A' && c <= 'Z') {
                    printf("%s ", morse_code[c - 'A']);
                } else if (c == ' ') {
                    printf("/ ");
                }
            }
        }
        printf("\n");
    } else {
        printf("Morse decode: (concept shown)\n");
        printf("Morse: %s\n", args[2]);
        printf("Note: Implement full decode for production\n");
    }
}

// ROT13 cipher
void do_rot13(char **args) {
    if (!args[1]) {
        printf("Usage: rot13 <text>\n");
        printf("Example: rot13 \"Hello World\"\n");
        return;
    }
    
    printf("Input:  ");
    for (int i = 1; args[i]; i++) {
        printf("%s ", args[i]);
    }
    printf("\nROT13:  ");
    
    for (int i = 1; args[i]; i++) {
        for (int j = 0; args[i][j]; j++) {
            char c = args[i][j];
            if (c >= 'A' && c <= 'Z') {
                c = ((c - 'A' + 13) % 26) + 'A';
            } else if (c >= 'a' && c <= 'z') {
                c = ((c - 'a' + 13) % 26) + 'a';
            }
            printf("%c", c);
        }
        printf(" ");
    }
    printf("\n");
}

// Reverse text
void do_reverse_text(char **args) {
    if (!args[1]) {
        printf("Usage: reverse <text>\n");
        return;
    }
    
    char text[1024] = "";
    for (int i = 1; args[i]; i++) {
        strcat(text, args[i]);
        if (args[i+1]) strcat(text, " ");
    }
    
    printf("Original: %s\n", text);
    printf("Reversed: ");
    for (int i = strlen(text) - 1; i >= 0; i--) {
        printf("%c", text[i]);
    }
    printf("\n");
}

// Binary conversion
void do_binary(char **args) {
    if (!args[1]) {
        printf("Usage: binary <decimal_number>\n");
        return;
    }
    
    long num = atol(args[1]);
    printf("Decimal: %ld\n", num);
    printf("Binary:  ");
    
    if (num == 0) {
        printf("0\n");
        return;
    }
    
    char binary[65] = "";
    int i = 0;
    long n = num < 0 ? -num : num;
    
    while (n > 0) {
        binary[i++] = (n % 2) + '0';
        n /= 2;
    }
    
    if (num < 0) printf("-");
    for (int j = i - 1; j >= 0; j--) {
        printf("%c", binary[j]);
    }
    printf("\n");
}

// Hex conversion
void do_hex(char **args) {
    if (!args[1]) {
        printf("Usage: hex <decimal_number>\n");
        return;
    }
    
    long num = atol(args[1]);
    printf("Decimal: %ld\n", num);
    printf("Hex:     0x%lX\n", num);
    printf("Hex:     0x%lx\n", num);
}

// Octal conversion
void do_octal(char **args) {
    if (!args[1]) {
        printf("Usage: octal <decimal_number>\n");
        return;
    }
    
    long num = atol(args[1]);
    printf("Decimal: %ld\n", num);
    printf("Octal:   0%lo\n", num);
}

// Base converter
void do_base_convert(char **args) {
    if (!args[1] || !args[2] || !args[3]) {
        printf("Usage: baseconv <number> <from_base> <to_base>\n");
        printf("Example: baseconv 255 10 16\n");
        return;
    }
    
    long num = strtol(args[1], NULL, atoi(args[2]));
    int to_base = atoi(args[3]);
    
    printf("From base %s: %s\n", args[2], args[1]);
    printf("Decimal value: %ld\n", num);
    
    if (to_base == 2) {
        printf("To base 2: ");
        char binary[65] = "";
        int i = 0;
        long n = num;
        while (n > 0) {
            binary[i++] = (n % 2) + '0';
            n /= 2;
        }
        for (int j = i - 1; j >= 0; j--) printf("%c", binary[j]);
        printf("\n");
    } else if (to_base == 8) {
        printf("To base 8: %lo\n", num);
    } else if (to_base == 16) {
        printf("To base 16: %lX\n", num);
    } else if (to_base == 10) {
        printf("To base 10: %ld\n", num);
    }
}

// Simple checksum
void do_checksum(char **args) {
    if (!args[1]) {
        printf("Usage: checksum <file>\n");
        return;
    }
    
    int fd = open(args[1], O_RDONLY);
    if (fd < 0) {
        perror("checksum");
        return;
    }
    
    unsigned long sum = 0, xor_sum = 0;
    unsigned char buf[4096];
    ssize_t n;
    
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            sum += buf[i];
            xor_sum ^= buf[i];
        }
    }
    close(fd);
    
    printf("File: %s\n", args[1]);
    printf("Simple Sum:  %lu\n", sum);
    printf("XOR Checksum: %lu (0x%lX)\n", xor_sum, xor_sum);
}

// CRC32 (simplified)
void do_crc32(char **args) {
    if (!args[1]) {
        printf("Usage: crc32 <file>\n");
        return;
    }
    
    printf("CRC32 for: %s\n", args[1]);
    printf("(Simplified implementation - concept shown)\n");
    do_checksum(args);
}

// Random number generator
void do_random(char **args) {
    int min = 1, max = 100;
    
    if (args[1]) min = atoi(args[1]);
    if (args[2]) max = atoi(args[2]);
    
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    
    srand(time(NULL) + getpid());
    int num = min + (rand() % (max - min + 1));
    
    printf("Random number [%d, %d]: %d\n", min, max, num);
}

// UUID generator (simplified)
void do_uuid(char **args) {
    (void)args;
    srand(time(NULL) + getpid());
    
    printf("UUID: ");
    printf("%04x%04x-", rand() & 0xFFFF, rand() & 0xFFFF);
    printf("%04x-", rand() & 0xFFFF);
    printf("4%03x-", rand() & 0x0FFF);
    printf("%04x-", (rand() & 0x3FFF) | 0x8000);
    printf("%04x%04x%04x\n", rand() & 0xFFFF, rand() & 0xFFFF, rand() & 0xFFFF);
}

// Timer
void do_timer(char **args) {
    if (!args[1]) {
        printf("Usage: timer <seconds>\n");
        return;
    }
    
    int seconds = atoi(args[1]);
    printf("Timer started for %d seconds...\n", seconds);
    
    for (int i = seconds; i > 0; i--) {
        printf("\r⏱  %02d:%02d remaining", i / 60, i % 60);
        fflush(stdout);
        sleep(1);
    }
    
    printf("\r✓ Timer complete!          \n");
}

// Stopwatch
void do_stopwatch(char **args) {
    (void)args;
    printf("Stopwatch: Press Ctrl+C to stop\n");
    printf("Started...\n");
    
    for (int i = 0; i < 3600; i++) {
        printf("\r⏱  %02d:%02d", i / 60, i % 60);
        fflush(stdout);
        sleep(1);
    }
    printf("\n");
}

// Color palette
void do_color_palette(char **args) {
    (void)args;
    printf("\n\033[1m\033[36m=== TERMINAL COLOR PALETTE ===\033[0m\n\n");
    
    printf("Standard Colors:\n");
    for (int i = 30; i <= 37; i++) {
        printf("  \033[%dm███\033[0m  Color %d    ", i, i);
        printf("\033[%d;1m███\033[0m  Bold %d\n", i, i);
    }
    
    printf("\nBackground Colors:\n");
    for (int i = 40; i <= 47; i++) {
        printf("  \033[%dm   \033[0m  BG %d      ", i, i);
    }
    printf("\n\n");
    
    printf("256 Color Sample:\n");
    for (int i = 0; i < 16; i++) {
        printf("\033[48;5;%dm  \033[0m", i);
    }
    printf("\n\n");
}

// ASCII art banner
void do_ascii_art(char **args) {
    if (!args[1]) {
        printf("Usage: ascii <text>\n");
        return;
    }
    
    printf("\n");
    // Simple ASCII art for letters
    for (int i = 1; args[i]; i++) {
        const char *word = args[i];
        for (int j = 0; word[j]; j++) {
            char c = toupper(word[j]);
            if (c == 'A') {
                printf("  /\\\\ ");
            } else if (c == 'B') {
                printf(" |==|");
            } else if (c == 'C') {
                printf("  /--");
            } else if (c == 'N') {
                printf(" |\\ |");
            } else if (c == 'L') {
                printf(" |   ");
            } else if (c == 'P') {
                printf(" |==.");
            } else {
                printf(" %c   ", c);
            }
        }
        printf("\n");
        
        for (int j = 0; word[j]; j++) {
            char c = toupper(word[j]);
            if (c == 'A') {
                printf(" /  \\\\");
            } else if (c == 'B') {
                printf(" |==|");
            } else if (c == 'C') {
                printf(" |   ");
            } else if (c == 'N') {
                printf(" | \\|");
            } else if (c == 'L') {
                printf(" |__ ");
            } else if (c == 'P') {
                printf(" |   ");
            } else {
                printf("     ");
            }
        }
        printf("\n\n");
    }
}

// Progress bar
void do_progress(char **args) {
    int percent = args[1] ? atoi(args[1]) : 100;
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    int bars = (percent * 50) / 100;
    
    printf("\n[");
    for (int j = 0; j < 50; j++) {
        printf(j < bars ? "█" : "░");
    }
    printf("] %3d%%\n", percent);
    
    if (percent == 100) {
        printf("✓ Complete!\n");
    }
    printf("\n");
}

// Lorem ipsum generator
void do_lorem(char **args) {
    int words = args[1] ? atoi(args[1]) : 50;
    
    const char *lorem_words[] = {
        "lorem", "ipsum", "dolor", "sit", "amet", "consectetur", "adipiscing",
        "elit", "sed", "do", "eiusmod", "tempor", "incididunt", "ut", "labore",
        "et", "dolore", "magna", "aliqua", "enim", "ad", "minim", "veniam",
        "quis", "nostrud", "exercitation", "ullamco", "laboris", "nisi",
        "aliquip", "ex", "ea", "commodo", "consequat"
    };
    int n_words = sizeof(lorem_words) / sizeof(lorem_words[0]);
    
    printf("Lorem Ipsum (%d words):\n\n", words);
    
    for (int i = 0; i < words; i++) {
        printf("%s ", lorem_words[i % n_words]);
        if ((i + 1) % 15 == 0) printf("\n");
    }
    printf("\n\n");
}

// Simple diff
void do_diff(char **args) {
    if (!args[1] || !args[2]) {
        printf("Usage: diff <file1> <file2>\n");
        return;
    }
    
    FILE *f1 = fopen(args[1], "r");
    FILE *f2 = fopen(args[2], "r");
    
    if (!f1 || !f2) {
        printf("Error opening files\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return;
    }
    
    printf("Comparing: %s <-> %s\n\n", args[1], args[2]);
    
    char line1[1024], line2[1024];
    int line_num = 1;
    int differences = 0;
    
    while (1) {
        char *r1 = fgets(line1, sizeof(line1), f1);
        char *r2 = fgets(line2, sizeof(line2), f2);
        
        if (!r1 && !r2) break;
        
        if (!r1) {
            printf("%d: + %s", line_num, line2);
            differences++;
        } else if (!r2) {
            printf("%d: - %s", line_num, line1);
            differences++;
        } else if (strcmp(line1, line2) != 0) {
            printf("%d:\n  - %s  + %s", line_num, line1, line2);
            differences++;
        }
        
        line_num++;
    }
    
    fclose(f1);
    fclose(f2);
    
    if (differences == 0) {
        printf("Files are identical.\n");
    } else {
        printf("\nTotal differences: %d lines\n", differences);
    }
}

// Compress (simple RLE)
void do_compress(char **args) {
    if (!args[1]) {
        printf("Usage: compress <file>\n");
        printf("Uses simple Run-Length Encoding (RLE)\n");
        return;
    }
    
    FILE *fp = fopen(args[1], "r");
    if (!fp) {
        perror("compress");
        return;
    }
    
    char outfile[512];
    snprintf(outfile, sizeof(outfile), "%s.rle", args[1]);
    FILE *out = fopen(outfile, "w");
    if (!out) {
        perror("compress");
        fclose(fp);
        return;
    }
    
    int ch, prev = -1, count = 0;
    
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == prev && count < 255) {
            count++;
        } else {
            if (prev != -1) {
                fprintf(out, "%c%c", (char)count, (char)prev);
            }
            prev = ch;
            count = 1;
        }
    }
    
    if (prev != -1) {
        fprintf(out, "%c%c", (char)count, (char)prev);
    }
    
    fclose(fp);
    fclose(out);
    
    printf("Compressed: %s -> %s\n", args[1], outfile);
}

// Decompress
void do_decompress(char **args) {
    if (!args[1]) {
        printf("Usage: decompress <file.rle>\n");
        return;
    }
    
    FILE *fp = fopen(args[1], "r");
    if (!fp) {
        perror("decompress");
        return;
    }
    
    // Remove .rle extension
    char outfile[512];
    strncpy(outfile, args[1], sizeof(outfile) - 1);
    char *ext = strstr(outfile, ".rle");
    if (ext) strcpy(ext, ".dec");
    else strcat(outfile, ".dec");
    
    FILE *out = fopen(outfile, "w");
    if (!out) {
        perror("decompress");
        fclose(fp);
        return;
    }
    
    int count, ch;
    while ((count = fgetc(fp)) != EOF) {
        ch = fgetc(fp);
        if (ch == EOF) break;
        for (int i = 0; i < count; i++) {
            fputc(ch, out);
        }
    }
    
    fclose(fp);
    fclose(out);
    
    printf("Decompressed: %s -> %s\n", args[1], outfile);
}

// Table generator (simplified)
void do_table(char **args) {
    (void)args;
    printf("\n\033[1m\033[36m=== TABLE EXAMPLE ===\033[0m\n\n");
    printf("  \033[1m%-12s  %-12s  %-12s\033[0m\n", "Header 1", "Header 2", "Header 3");
    printf("  %-12s  %-12s  %-12s\n", "------------", "------------", "------------");
    printf("  %-12s  %-12s  %-12s\n", "Data 1", "Data 2", "Data 3");
    printf("  %-12s  %-12s  %-12s\n", "Data 4", "Data 5", "Data 6");
    printf("\n\033[2m(Concept shown - extend for dynamic tables)\033[0m\n");
}

// JSON formatter (simplified)
void do_json_format(char **args) {
    if (!args[1]) {
        printf("Usage: jsonformat <json_string>\n");
        return;
    }
    
    printf("JSON Formatter (simplified):\n");
    int indent = 0;
    
    for (int i = 1; args[i]; i++) {
        for (int j = 0; args[i][j]; j++) {
            char c = args[i][j];
            
            if (c == '{' || c == '[') {
                printf("%c\n", c);
                indent += 2;
                for (int k = 0; k < indent; k++) printf(" ");
            } else if (c == '}' || c == ']') {
                printf("\n");
                indent -= 2;
                for (int k = 0; k < indent; k++) printf(" ");
                printf("%c", c);
            } else if (c == ',') {
                printf("%c\n", c);
                for (int k = 0; k < indent; k++) printf(" ");
            } else {
                printf("%c", c);
            }
        }
    }
    printf("\n");
}
