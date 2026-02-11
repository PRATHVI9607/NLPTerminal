/**
 * Expression Converter and Evaluator
 * Handles infix, postfix, and prefix notation conversions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "expression.h"

#define MAX_STACK 100
#define MAX_EXPR 512
#define MAX_RESULT (MAX_EXPR * 2 + 10)  // For combining two expressions

// Helper function to safely concatenate arguments
static void safe_concat_args(char *dest, size_t dest_size, char **args, int start_index) {
    dest[0] = '\0';
    size_t current_len = 0;
    for (int i = start_index; args[i]; i++) {
        size_t arg_len = strlen(args[i]);
        if (current_len + arg_len + 2 < dest_size) {
            strcat(dest, args[i]);
            current_len += arg_len;
            if (args[i+1]) {
                strcat(dest, " ");
                current_len++;
            }
        }
    }
}

// Helper function to strip quotes from string
static void strip_quotes(char *str) {
    int len = strlen(str);
    if (len >= 2 && str[0] == '"' && str[len-1] == '"') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

// Stack for characters
typedef struct {
    char items[MAX_STACK];
    int top;
} CharStack;

// Stack for strings
typedef struct {
    char items[MAX_STACK][MAX_EXPR];
    int top;
} StringStack;

// Stack for numbers
typedef struct {
    double items[MAX_STACK];
    int top;
} NumStack;

// Initialize stacks
static void init_char_stack(CharStack *s) { s->top = -1; }
static void init_string_stack(StringStack *s) { s->top = -1; }
static void init_num_stack(NumStack *s) { s->top = -1; }

// Stack operations
static void push_char(CharStack *s, char c) { if (s->top < MAX_STACK - 1) s->items[++(s->top)] = c; }
static char pop_char(CharStack *s) { return (s->top >= 0) ? s->items[(s->top)--] : '\0'; }
static char peek_char(CharStack *s) { return (s->top >= 0) ? s->items[s->top] : '\0'; }
static int is_empty_char(CharStack *s) { return s->top == -1; }

static void push_string(StringStack *s, const char *str) {
    if (s->top < MAX_STACK - 1) strcpy(s->items[++(s->top)], str);
}
static void pop_string(StringStack *s, char *out) {
    if (s->top >= 0) strcpy(out, s->items[(s->top)--]);
    else out[0] = '\0';
}

static void push_num(NumStack *s, double n) { if (s->top < MAX_STACK - 1) s->items[++(s->top)] = n; }
static double pop_num(NumStack *s) { return (s->top >= 0) ? s->items[(s->top)--] : 0.0; }

// Check if character is operator
static int is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%';
}

// Get operator precedence
static int precedence(char op) {
    if (op == '^') return 3;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

// Check if operator is right associative
static int is_right_assoc(char op) {
    return op == '^';
}

// Infix to Postfix conversion
void do_infix2postfix(char **args) {
    if (!args[1]) {
        printf("Usage: infix2postfix <expression>\n");
        printf("Example: infix2postfix \"a+b*c\"\n");
        return;
    }
    
    // Concatenate all arguments
    char infix[MAX_EXPR];
    safe_concat_args(infix, MAX_EXPR, args, 1);
    strip_quotes(infix);
    
    CharStack stack;
    init_char_stack(&stack);
    char postfix[MAX_EXPR] = "";
    int pos = 0;
    
    for (int i = 0; infix[i]; i++) {
        char c = infix[i];
        
        if (isspace(c)) continue;
        
        if (isalnum(c) || c == '.') {
            // Operand
            postfix[pos++] = c;
            // If next char is not alnum or ., add space
            if (!isalnum(infix[i+1]) && infix[i+1] != '.') {
                postfix[pos++] = ' ';
            }
        }
        else if (c == '(') {
            push_char(&stack, c);
        }
        else if (c == ')') {
            while (!is_empty_char(&stack) && peek_char(&stack) != '(') {
                postfix[pos++] = pop_char(&stack);
                postfix[pos++] = ' ';
            }
            if (!is_empty_char(&stack)) pop_char(&stack); // Remove '('
        }
        else if (is_operator(c)) {
            while (!is_empty_char(&stack) && peek_char(&stack) != '(' &&
                   (precedence(peek_char(&stack)) > precedence(c) ||
                    (precedence(peek_char(&stack)) == precedence(c) && !is_right_assoc(c)))) {
                postfix[pos++] = pop_char(&stack);
                postfix[pos++] = ' ';
            }
            push_char(&stack, c);
        }
    }
    
    while (!is_empty_char(&stack)) {
        postfix[pos++] = pop_char(&stack);
        postfix[pos++] = ' ';
    }
    
    postfix[pos] = '\0';
    
    printf("Infix:    %s\n", infix);
    printf("Postfix:  %s\n", postfix);
}

// Infix to Prefix conversion
void do_infix2prefix(char **args) {
    if (!args[1]) {
        printf("Usage: infix2prefix <expression>\n");
        printf("Example: infix2prefix \"a+b*c\"\n");
        return;
    }
    
    // Concatenate and reverse
    char infix[MAX_EXPR];
    safe_concat_args(infix, MAX_EXPR, args, 1);
    strip_quotes(infix);
    
    // Reverse the infix and swap parentheses
    int len = strlen(infix);
    char reversed[MAX_EXPR];
    for (int i = 0; i < len; i++) {
        if (infix[len-1-i] == '(') reversed[i] = ')';
        else if (infix[len-1-i] == ')') reversed[i] = '(';
        else reversed[i] = infix[len-1-i];
    }
    reversed[len] = '\0';
    
    // Convert to postfix
    CharStack stack;
    init_char_stack(&stack);
    char postfix[MAX_EXPR] = "";
    int pos = 0;
    
    for (int i = 0; reversed[i]; i++) {
        char c = reversed[i];
        
        if (isspace(c)) continue;
        
        if (isalnum(c) || c == '.') {
            postfix[pos++] = c;
            if (!isalnum(reversed[i+1]) && reversed[i+1] != '.') {
                postfix[pos++] = ' ';
            }
        }
        else if (c == '(') {
            push_char(&stack, c);
        }
        else if (c == ')') {
            while (!is_empty_char(&stack) && peek_char(&stack) != '(') {
                postfix[pos++] = pop_char(&stack);
                postfix[pos++] = ' ';
            }
            if (!is_empty_char(&stack)) pop_char(&stack);
        }
        else if (is_operator(c)) {
            while (!is_empty_char(&stack) && peek_char(&stack) != '(' &&
                   precedence(peek_char(&stack)) >= precedence(c)) {
                postfix[pos++] = pop_char(&stack);
                postfix[pos++] = ' ';
            }
            push_char(&stack, c);
        }
    }
    
    while (!is_empty_char(&stack)) {
        postfix[pos++] = pop_char(&stack);
        postfix[pos++] = ' ';
    }
    postfix[pos] = '\0';
    
    // Reverse the postfix to get prefix
    len = strlen(postfix);
    char prefix[MAX_EXPR];
    for (int i = 0; i < len; i++) {
        prefix[i] = postfix[len-1-i];
    }
    prefix[len] = '\0';
    
    printf("Infix:   %s\n", infix);
    printf("Prefix:  %s\n", prefix);
}

// Postfix to Infix conversion
void do_postfix2infix(char **args) {
    if (!args[1]) {
        printf("Usage: postfix2infix <expression>\n");
        printf("Example: postfix2infix \"ab+c*\"\n");
        return;
    }
    
    char postfix[MAX_EXPR];
    safe_concat_args(postfix, MAX_EXPR, args, 1);
    strip_quotes(postfix);
    
    StringStack stack;
    init_string_stack(&stack);
    
    char token[MAX_EXPR];
    int j = 0;
    
    for (int i = 0; postfix[i]; i++) {
        if (isspace(postfix[i])) {
            if (j > 0) {
                token[j] = '\0';
                j = 0;
                
                if (is_operator(token[0]) && strlen(token) == 1) {
                    char op2[MAX_EXPR], op1[MAX_EXPR];
                    pop_string(&stack, op2);
                    pop_string(&stack, op1);
                    char result[MAX_RESULT];
                    snprintf(result, sizeof(result), "(%s %c %s)", op1, token[0], op2);
                    push_string(&stack, result);
                } else {
                    push_string(&stack, token);
                }
            }
        } else {
            token[j++] = postfix[i];
        }
    }
    
    // Process last token
    if (j > 0) {
        token[j] = '\0';
        if (is_operator(token[0]) && strlen(token) == 1) {
            char op2[MAX_EXPR], op1[MAX_EXPR];
            pop_string(&stack, op2);
            pop_string(&stack, op1);
            char result[MAX_RESULT];
            snprintf(result, sizeof(result), "(%s %c %s)", op1, token[0], op2);
            push_string(&stack, result);
        } else {
            push_string(&stack, token);
        }
    }
    
    char infix[MAX_EXPR];
    pop_string(&stack, infix);
    
    printf("Postfix: %s\n", postfix);
    printf("Infix:   %s\n", infix);
}

// Prefix to Infix conversion
void do_prefix2infix(char **args) {
    if (!args[1]) {
        printf("Usage: prefix2infix <expression>\n");
        printf("Example: prefix2infix \"+*abc\"\n");
        return;
    }
    
    char prefix[MAX_EXPR];
    safe_concat_args(prefix, MAX_EXPR, args, 1);
    strip_quotes(prefix);
    
    StringStack stack;
    init_string_stack(&stack);
    
    // Process from right to left
    int len = strlen(prefix);
    char token[MAX_EXPR];
    
    for (int i = len - 1; i >= 0; i--) {
        if (isspace(prefix[i])) continue;
        
        if (isalnum(prefix[i])) {
            // Collect full operand (could be multi-digit)
            int j = i;
            while (j >= 0 && isalnum(prefix[j])) j--;
            j++;
            
            int k = 0;
            for (int m = j; m <= i; m++) {
                token[k++] = prefix[m];
            }
            token[k] = '\0';
            
            push_string(&stack, token);
            i = j;  // Skip processed characters
        }
        else if (is_operator(prefix[i])) {
            char op1[MAX_EXPR], op2[MAX_EXPR];
            pop_string(&stack, op1);
            pop_string(&stack, op2);
            char result[MAX_RESULT];
            snprintf(result, sizeof(result), "(%s %c %s)", op1, prefix[i], op2);
            push_string(&stack, result);
        }
    }
    
    char infix[MAX_EXPR];
    pop_string(&stack, infix);
    
    printf("Prefix:  %s\n", prefix);
    printf("Infix:   %s\n", infix);
}

// Evaluate postfix expression
void do_evaluate(char **args) {
    if (!args[1]) {
        printf("Usage: evaluate <postfix_expression>\n");
        printf("Example: evaluate \"5 3 +\"\n");
        return;
    }
    
    char expr[MAX_EXPR];
    safe_concat_args(expr, MAX_EXPR, args, 1);
    strip_quotes(expr);
    
    NumStack stack;
    init_num_stack(&stack);
    
    char token[MAX_EXPR];
    int j = 0;
    
    for (int i = 0; expr[i]; i++) {
        if (isspace(expr[i])) {
            if (j > 0) {
                token[j] = '\0';
                j = 0;
                
                if (is_operator(token[0]) && strlen(token) == 1) {
                    double b = pop_num(&stack);
                    double a = pop_num(&stack);
                    double result;
                    
                    switch (token[0]) {
                        case '+': result = a + b; break;
                        case '-': result = a - b; break;
                        case '*': result = a * b; break;
                        case '/': 
                            if (b == 0) {
                                printf("Error: Division by zero\n");
                                return;
                            }
                            result = a / b;
                            break;
                        case '^': result = pow(a, b); break;
                        case '%': result = (int)a % (int)b; break;
                        default: result = 0; break;
                    }
                    push_num(&stack, result);
                } else {
                    push_num(&stack, atof(token));
                }
            }
        } else {
            token[j++] = expr[i];
        }
    }
    
    // Process last token
    if (j > 0) {
        token[j] = '\0';
        if (is_operator(token[0]) && strlen(token) == 1) {
            double b = pop_num(&stack);
            double a = pop_num(&stack);
            double result;
            
            switch (token[0]) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': result = b != 0 ? a / b : 0; break;
                case '^': result = pow(a, b); break;
                case '%': result = (int)a % (int)b; break;
                default: result = 0; break;
            }
            push_num(&stack, result);
        } else {
            push_num(&stack, atof(token));
        }
    }
    
    double result = pop_num(&stack);
    printf("Expression: %s\n", expr);
    printf("Result:     %.6g\n", result);
}

// Detect expression type
void do_exprtype(char **args) {
    if (!args[1]) {
        printf("Usage: exprtype <expression>\n");
        printf("Detects if expression is infix, postfix, or prefix\n");
        return;
    }
    
    char expr[MAX_EXPR];
    safe_concat_args(expr, MAX_EXPR, args, 1);
    strip_quotes(expr);
    
    int has_paren = 0, operator_count = 0, operand_count = 0;
    int operator_first = 0, operator_last = 0;
    
    for (int i = 0; expr[i]; i++) {
        if (expr[i] == '(' || expr[i] == ')') has_paren = 1;
        if (is_operator(expr[i])) {
            operator_count++;
            if (i == 0) operator_first = 1;
            // Check if last non-space char
            int j = i + 1;
            while (expr[j] && isspace(expr[j])) j++;
            if (!expr[j]) operator_last = 1;
        }
        if (isalnum(expr[i])) operand_count++;
    }
    
    printf("Expression: %s\n", expr);
    printf("Analysis:\n");
    printf("  Operators: %d\n", operator_count);
    printf("  Operands:  ~%d\n", operand_count);
    printf("  Parentheses: %s\n", has_paren ? "Yes" : "No");
    
    if (has_paren) {
        printf("  Type: Likely INFIX (contains parentheses)\n");
    } else if (operator_first) {
        printf("  Type: Likely PREFIX (operator first)\n");
    } else if (operator_last) {
        printf("  Type: Likely POSTFIX (operator last)\n");
    } else {
        printf("  Type: Likely INFIX (mixed operators and operands)\n");
    }
}
// Add this to the end of expression.c

// Helper to visualize stack
static void print_char_stack(CharStack *stack, const char *label) {
    printf("  Stack (%s): [ ", label);
    for (int i = 0; i <= stack->top; i++) {
        if (stack->items[i] == '\0') break;
        printf("'%c' ", stack->items[i]);
    }
    printf("]\n");
}

static void print_string_stack(StringStack *stack, const char *label) {
    printf("  Stack (%s): [ ", label);
    for (int i = 0; i <= stack->top; i++) {
        printf("\"%s\" ", stack->items[i]);
    }
    printf("]\n");
}

static void print_num_stack(NumStack *stack, const char *label) {
    printf("  Stack (%s): [ ", label);
    for (int i = 0; i <= stack->top; i++) {
        printf("%.2f ", stack->items[i]);
    }
    printf("]\n");
}

// Visualized Infix to Postfix
void do_infix2postfix_visual(char **args) {
    if (!args[1]) {
        printf("Usage: visualfix <expression>\n");
        printf("Example: visualfix \"a+b*c\"\n");
        printf("Shows stack operations step-by-step\n");
        return;
    }
    
    char infix[MAX_EXPR];
    safe_concat_args(infix, MAX_EXPR, args, 1);
    strip_quotes(infix);
    
    printf("\n\033[1m\033[36m=== INFIX TO POSTFIX CONVERSION ===\033[0m\n\n");
    printf("Expression: %s\n\n", infix);
    
    CharStack stack;
    init_char_stack(&stack);
    char postfix[MAX_EXPR] = "";
    int pos = 0;
    int step = 1;
    
    printf("\033[1m%-6s %-10s %-14s %-25s\033[0m\n", "Step", "Char", "Action", "Operator Stack");
    printf("%-6s %-10s %-14s %-25s\n", "----", "----", "------", "--------------");
    
    for (int i = 0; infix[i]; i++) {
        char c = infix[i];
        
        if (isspace(c)) continue;
        
        printf("%-6d %-10c ", step++, c);
        
        if (isalnum(c) || c == '.') {
            printf("%-14s ", "Output");
            postfix[pos++] = c;
            if (!isalnum(infix[i+1]) && infix[i+1] != '.') {
                postfix[pos++] = ' ';
            }
            print_char_stack(&stack, "");
        }
        else if (c == '(') {
            printf("%-14s ", "Push '('");
            push_char(&stack, c);
            print_char_stack(&stack, "");
        }
        else if (c == ')') {
            printf("%-14s ", "Pop till '('");
            while (!is_empty_char(&stack) && peek_char(&stack) != '(') {
                postfix[pos++] = pop_char(&stack);
                postfix[pos++] = ' ';
            }
            if (!is_empty_char(&stack)) pop_char(&stack);
            print_char_stack(&stack, "");
        }
        else if (is_operator(c)) {
            printf("%-14s ", "Push/Pop ops");
            while (!is_empty_char(&stack) && peek_char(&stack) != '(' &&
                   (precedence(peek_char(&stack)) > precedence(c) ||
                    (precedence(peek_char(&stack)) == precedence(c) && !is_right_assoc(c)))) {
                postfix[pos++] = pop_char(&stack);
                postfix[pos++] = ' ';
            }
            push_char(&stack, c);
            print_char_stack(&stack, "");
        }
    }
    
    printf("%-6s %-10s %-14s ", "...", "(END)", "Pop all");
    while (!is_empty_char(&stack)) {
        postfix[pos++] = pop_char(&stack);
        postfix[pos++] = ' ';
    }
    printf("[ ]\n\n");
    
    postfix[pos] = '\0';
    
    printf("\033[1m\033[32mResult:\033[0m %s\n\n", postfix);
    
    printf("Data Structure: STACK (LIFO - Last In First Out)\n");
    printf("Algorithm: Shunting Yard Algorithm by Dijkstra\n\n");
}

// Visualized Postfix to Infix
void do_postfix2infix_visual(char **args) {
    if (!args[1]) {
        printf("Usage: visualpost <expression>\n");
        printf("Example: visualpost \"ab+c*\"\n");
        return;
    }
    
    char postfix[MAX_EXPR];
    safe_concat_args(postfix, MAX_EXPR, args, 1);
    strip_quotes(postfix);
    
    printf("\n\033[1m\033[36m=== POSTFIX TO INFIX CONVERSION ===\033[0m\n\n");
    printf("Expression: %s\n\n", postfix);
    
    StringStack stack;
    init_string_stack(&stack);
    
    printf("\033[1m%-6s %-10s %-45s\033[0m\n", "Step", "Token", "Expression Stack");
    printf("%-6s %-10s %-45s\n", "----", "-----", "----------------");
    
    char token[MAX_EXPR];
    int j = 0;
    int step = 1;
    
    for (int i = 0; postfix[i]; i++) {
        if (isspace(postfix[i])) {
            if (j > 0) {
                token[j] = '\0';
                j = 0;
                
                printf("%-6d %-10s ", step++, token);
                
                if (is_operator(token[0]) && strlen(token) == 1) {
                    char op2[MAX_EXPR], op1[MAX_EXPR];
                    pop_string(&stack, op2);
                    pop_string(&stack, op1);
                    char result[MAX_RESULT];
                    snprintf(result, sizeof(result), "(%s %c %s)", op1, token[0], op2);
                    push_string(&stack, result);
                    printf("Combine: %s\n", result);
                } else {
                    push_string(&stack, token);
                    printf("Push: %s\n", token);
                }
                
                printf("%-6s %-10s ", "", "");
                print_string_stack(&stack, "");
            }
        } else {
            token[j++] = postfix[i];
        }
    }
    
    // Process last token
    if (j > 0) {
        token[j] = '\0';
        printf("%-6d %-10s ", step++, token);
        
        if (is_operator(token[0]) && strlen(token) == 1) {
            char op2[MAX_EXPR], op1[MAX_EXPR];
            pop_string(&stack, op2);
            pop_string(&stack, op1);
            char result[MAX_RESULT];
            snprintf(result, sizeof(result), "(%s %c %s)", op1, token[0], op2);
            push_string(&stack, result);
            printf("Combine: %s\n", result);
        } else {
            push_string(&stack, token);
            printf("Push: %s\n", token);
        }
        
        printf("%-6s %-10s ", "", "");
        print_string_stack(&stack, "");
    }
    
    printf("\n");
    
    char infix[MAX_EXPR];
    pop_string(&stack, infix);
    
    printf("\033[1m\033[32mResult:\033[0m %s\n\n", infix);
    
    printf("\033[2mData Structure: STACK of STRINGS\033[0m\n");
    printf("\033[2mEach operator combines top two expressions into one\033[0m\n\n");
}

// Helper function to detect if expression is infix (has operator between operands)
static int is_infix_expression(const char *expr) {
    // Check for parentheses - definitely infix
    if (strchr(expr, '(') || strchr(expr, ')')) return 1;
    
    // Split into tokens and check pattern
    char temp[MAX_EXPR];
    strncpy(temp, expr, MAX_EXPR - 1);
    temp[MAX_EXPR - 1] = '\0';
    
    char *tokens[100];
    int count = 0;
    char *tok = strtok(temp, " \t");
    while (tok && count < 100) {
        tokens[count++] = tok;
        tok = strtok(NULL, " \t");
    }
    
    if (count < 2) return 0;
    
    // Check for pattern: operand operator operand operator ...
    // Infix: 5 + 3 * 2 (operator between operands)
    // Postfix: 5 3 + 2 * (operators after operands)
    // Prefix: + 5 * 3 2 (operators before operands)
    
    int last_was_num = 0;
    for (int i = 0; i < count; i++) {
        int is_op = (strlen(tokens[i]) == 1 && is_operator(tokens[i][0]));
        int is_num = !is_op;
        
        // In infix, operators come BETWEEN operands (num op num)
        // If we see num followed by op followed by num, it's likely infix
        if (i > 0 && i < count - 1) {
            if (is_op && last_was_num) {
                // Check if next is also a number
                int next_is_op = (strlen(tokens[i+1]) == 1 && is_operator(tokens[i+1][0]));
                if (!next_is_op) return 1;  // num op num -> infix
            }
        }
        last_was_num = is_num;
    }
    
    return 0;
}

// Helper function to convert infix to postfix (returns allocated string)
static char* convert_infix_to_postfix(const char *infix) {
    static char postfix[MAX_EXPR];
    CharStack stack;
    init_char_stack(&stack);
    int pos = 0;
    
    for (int i = 0; infix[i]; i++) {
        char c = infix[i];
        
        if (isspace(c)) continue;
        
        if (isalnum(c) || c == '.') {
            postfix[pos++] = c;
            if (!isalnum(infix[i+1]) && infix[i+1] != '.') {
                postfix[pos++] = ' ';
            }
        }
        else if (c == '(') {
            push_char(&stack, c);
        }
        else if (c == ')') {
            while (!is_empty_char(&stack) && peek_char(&stack) != '(') {
                postfix[pos++] = pop_char(&stack);
                postfix[pos++] = ' ';
            }
            if (!is_empty_char(&stack)) pop_char(&stack);
        }
        else if (is_operator(c)) {
            while (!is_empty_char(&stack) && peek_char(&stack) != '(' &&
                   (precedence(peek_char(&stack)) > precedence(c) ||
                    (precedence(peek_char(&stack)) == precedence(c) && !is_right_assoc(c)))) {
                postfix[pos++] = pop_char(&stack);
                postfix[pos++] = ' ';
            }
            push_char(&stack, c);
        }
    }
    
    while (!is_empty_char(&stack)) {
        postfix[pos++] = pop_char(&stack);
        postfix[pos++] = ' ';
    }
    
    postfix[pos] = '\0';
    return postfix;
}

// Visualized Postfix Evaluation (with auto-detection for infix)
void do_evaluate_visual(char **args) {
    if (!args[1]) {
        printf("Usage: visualeval <expression>\n");
        printf("Example: visualeval \"5 3 + 2 *\"  (postfix)\n");
        printf("         visualeval \"5 + 3 * 2\"  (infix - auto-converted)\n");
        return;
    }
    
    char expr[MAX_EXPR];
    safe_concat_args(expr, MAX_EXPR, args, 1);
    strip_quotes(expr);
    
    // Auto-detect if infix and convert
    char *eval_expr = expr;
    
    if (is_infix_expression(expr)) {
        printf("\n\033[1m\033[33m[Auto-detected INFIX - Converting to Postfix]\033[0m\n");
        printf("Original (Infix):    %s\n", expr);
        eval_expr = convert_infix_to_postfix(expr);
        printf("Converted (Postfix): %s\n", eval_expr);
    }
    
    printf("\n\033[1m\033[36m=== POSTFIX EVALUATION ===\033[0m\n\n");
    printf("Expression: %s\n\n", eval_expr);
    
    NumStack stack;
    init_num_stack(&stack);
    
    printf("\033[1m%-6s %-10s %-14s %-26s\033[0m\n", "Step", "Token", "Operation", "Number Stack");
    printf("%-6s %-10s %-14s %-26s\n", "----", "-----", "---------", "------------");
    
    char token[MAX_EXPR];
    int j = 0;
    int step = 1;
    
    for (int i = 0; eval_expr[i]; i++) {
        if (isspace(eval_expr[i])) {
            if (j > 0) {
                token[j] = '\0';
                j = 0;
                
                printf("%-6d %-10s ", step++, token);
                
                if (is_operator(token[0]) && strlen(token) == 1) {
                    double b = pop_num(&stack);
                    double a = pop_num(&stack);
                    double result;
                    
                    switch (token[0]) {
                        case '+': result = a + b; printf("%-14s ", "add"); break;
                        case '-': result = a - b; printf("%-14s ", "subtract"); break;
                        case '*': result = a * b; printf("%-14s ", "multiply"); break;
                        case '/': 
                            if (b == 0) {
                                printf("\033[31mERROR: Division by zero\033[0m\n");
                                return;
                            }
                            result = a / b;
                            printf("%-14s ", "divide");
                            break;
                        case '^': result = pow(a, b); printf("%-14s ", "power"); break;
                        case '%': result = (int)a % (int)b; printf("%-14s ", "modulo"); break;
                        default: result = 0; break;
                    }
                    push_num(&stack, result);
                    print_num_stack(&stack, "");
                } else {
                    double num = atof(token);
                    push_num(&stack, num);
                    printf("%-14s ", "push");
                    print_num_stack(&stack, "");
                }
            }
        } else {
            token[j++] = eval_expr[i];
        }
    }
    
    // Process last token
    if (j > 0) {
        token[j] = '\0';
        printf("%-6d %-10s ", step++, token);
        
        if (is_operator(token[0]) && strlen(token) == 1) {
            double b = pop_num(&stack);
            double a = pop_num(&stack);
            double result;
            
            switch (token[0]) {
                case '+': result = a + b; printf("%-14s ", "add"); break;
                case '-': result = a - b; printf("%-14s ", "subtract"); break;
                case '*': result = a * b; printf("%-14s ", "multiply"); break;
                case '/': result = b != 0 ? a / b : 0; printf("%-14s ", "divide"); break;
                case '^': result = pow(a, b); printf("%-14s ", "power"); break;
                case '%': result = (int)a % (int)b; printf("%-14s ", "modulo"); break;
                default: result = 0; break;
            }
            push_num(&stack, result);
            print_num_stack(&stack, "");
        } else {
            double num = atof(token);
            push_num(&stack, num);
            printf("%-14s ", "push");
            print_num_stack(&stack, "");
        }
    }
    
    printf("\n");
    
    double result = pop_num(&stack);
    
    printf("\033[1m\033[32mFINAL RESULT: %.6g\033[0m\n\n", result);
    
    printf("\033[2mData Structure: STACK of NUMBERS (doubles)\033[0m\n");
    printf("\033[2mAlgorithm: Standard Postfix Evaluation\033[0m\n");
    printf("\033[2m  - Operands are pushed onto stack\033[0m\n");
    printf("\033[2m  - Operators pop two values, compute, push result\033[0m\n\n");
}
