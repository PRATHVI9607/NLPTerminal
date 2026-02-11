/**
 * Expression Converter and Evaluator
 * Converts between infix, postfix, and prefix notations
 * Evaluates expressions
 */

#ifndef EXPRESSION_H
#define EXPRESSION_H

// Convert infix to postfix
void do_infix2postfix(char **args);

// Convert infix to prefix
void do_infix2prefix(char **args);

// Convert postfix to infix
void do_postfix2infix(char **args);

// Convert prefix to infix
void do_prefix2infix(char **args);

// Evaluate postfix expression
void do_evaluate(char **args);

// Expression type detector
void do_exprtype(char **args);

// Visualized conversions (show stack at each step)
void do_infix2postfix_visual(char **args);
void do_postfix2infix_visual(char **args);
void do_evaluate_visual(char **args);

#endif
