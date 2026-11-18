/*
 * calculator.c
 *
 * 基于 C 语言栈与中缀表达式的简易加减乘除计算器
 * ---------------------------------------------------------------
 * 知识点:
 *   1. 运算符栈 (char op_stack) 与操作数栈 (double num_stack)
 *   2. 中缀表达式 -> 后缀表达式思路 (Shunting-yard 调度场算法),
 *      边扫描边按优先级弹出运算符栈并计算
 *   3. 支持 + - * / 、括号 ( ) 、小数以及一元负号 (如 -3 + 2)
 *
 * 界面: 仅使用 printf 打印 ASCII 简易计算器面板, 不依赖任何图形库。
 *
 * 编译: gcc -std=c11 -Wall -Wextra -o calculator calculator.c
 * 运行: ./calculator   (Windows: calculator.exe)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_EXPR   256     /* 输入表达式最大长度 */
#define STACK_SIZE 128     /* 两个栈的最大容量  */

/* ===================== 运算符栈 ===================== */

char op_stack[STACK_SIZE];
int  op_top = -1;

static void op_push(char c)
{
    op_stack[++op_top] = c;
}

static char op_pop(void)
{
    return op_stack[op_top--];
}

static char op_peek(void)
{
    return op_stack[op_top];
}

static int op_empty(void)
{
    return op_top == -1;
}

/* ===================== 操作数栈 ===================== */

double num_stack[STACK_SIZE];
int    num_top = -1;

static void num_push(double v)
{
    num_stack[++num_top] = v;
}

static double num_pop(void)
{
    return num_stack[num_top--];
}

/* ===================== 计算逻辑 ===================== */

/* 运算符优先级: * / 高于 + - */
static int precedence(char op)
{
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

/* 执行一次二元运算, 成功返回 1, 除零/非法返回 0 */
static int apply_op(char op, double a, double b, double *result)
{
    switch (op) {
        case '+': *result = a + b; return 1;
        case '-': *result = a - b; return 1;
        case '*': *result = a * b; return 1;
        case '/':
            if (b == 0.0) return 0;          /* 除数为 0 */
            *result = a / b;
            return 1;
    }
    return 0;
}

/* 弹出栈顶运算符并计算, 成功返回 1 */
static int evaluate_top(void)
{
    char   op = op_pop();
    double b  = num_pop();
    double a  = num_pop();
    double r;
    if (!apply_op(op, a, b, &r)) return 0;
    num_push(r);
    return 1;
}

/*
 * 计算中缀表达式
 * 成功: 返回结果, *ok 置 1
 * 失败: *ok 置 0 (括号不匹配 / 非法字符 / 除零 / 操作数个数异常)
 */
static double evaluate(const char *expr, int *ok)
{
    *ok = 1;
    op_top = -1;
    num_top = -1;

    int  i = 0;
    int  n = (int)strlen(expr);
    int  expect_operand = 1;   /* 下一个字符是否应为操作数/左括号/一元负号 */

    while (i < n) {
        char c = expr[i];

        if (isspace((unsigned char)c)) { i++; continue; }

        /* ---- 数字 (含小数) ---- */
        if (isdigit((unsigned char)c) || c == '.') {
            double v = strtod(expr + i, NULL);
            num_push(v);
            expect_operand = 0;
            while (i < n && (isdigit((unsigned char)expr[i]) || expr[i] == '.')) {
                i++;
            }
            continue;
        }

        /* ---- 左括号 ---- */
        if (c == '(') {
            op_push(c);
            i++;
            expect_operand = 1;
            continue;
        }

        /* ---- 右括号: 弹出直到左括号 ---- */
        if (c == ')') {
            int found = 0;
            while (!op_empty()) {
                if (op_peek() == '(') { found = 1; break; }
                if (!evaluate_top()) { *ok = 0; return 0; }
            }
            if (!found) { *ok = 0; return 0; }   /* 缺少左括号 */
            op_pop();                             /* 丢弃 '(' */
            i++;
            expect_operand = 0;
            continue;
        }

        /* ---- 四则运算符 ---- */
        if (c == '+' || c == '-' || c == '*' || c == '/') {
            /* 一元正号 / 负号: 在表达式开头、左括号后或另一运算符后出现 */
            if (expect_operand && (c == '-' || c == '+')) {
                num_push(0.0);                    /* 0 - x 等价于 -x */
            }
            /* 栈顶运算符优先级不低于当前运算符时, 先弹出计算 */
            while (!op_empty() && op_peek() != '(' &&
                   precedence(op_peek()) >= precedence(c)) {
                if (!evaluate_top()) { *ok = 0; return 0; }
            }
            op_push(c);
            i++;
            expect_operand = 1;
            continue;
        }

        /* 非法字符 */
        *ok = 0;
        return 0;
    }

    /* ---- 扫描结束, 清空运算符栈 ---- */
    while (!op_empty()) {
        if (op_peek() == '(') { *ok = 0; return 0; }  /* 缺少右括号 */
        if (!evaluate_top()) { *ok = 0; return 0; }
    }

    if (num_top != 0) { *ok = 0; return 0; }          /* 操作数个数异常 */
    return num_pop();
}

/* ===================== printf 界面 ===================== */

/* 仅用 printf 绘制 ASCII 简易计算器面板 */
static void draw_screen(const char *expr, const char *result)
{
    printf("\n");
    printf("  +------------------------------------------+\n");
    printf("  |      C Stack Infix Calculator           |\n");
    printf("  +------------------------------------------+\n");
    printf("  |  Expression : %-24s |\n", expr);
    printf("  |  Result     : %-24s |\n", result);
    printf("  +------------------------------------------+\n");
    printf("  |  [7] [8] [9] [ / ]    [ ( ] [ ) ]       |\n");
    printf("  |  [4] [5] [6] [ * ]                      |\n");
    printf("  |  [1] [2] [3] [ - ]                      |\n");
    printf("  |  [0] [ . ] [ = ] [ + ]                  |\n");
    printf("  +------------------------------------------+\n");
    printf("  |  Operators: + - * / ( )    |  q : quit |\n");
    printf("  +------------------------------------------+\n");
}

/* ===================== 主流程 ===================== */

int main(void)
{
    char expr[MAX_EXPR]  = "";
    char result[64]      = "";

    printf("====== C Stack Calculator ======\n");

    while (1) {
        draw_screen(expr, result);

        printf("\n  Enter expression > ");
        if (!fgets(expr, sizeof(expr), stdin)) break;

        /* 去掉行尾换行符 */
        expr[strcspn(expr, "\r\n")] = '\0';

        if (expr[0] == 'q' || expr[0] == 'Q') {
            printf("  Bye!\n");
            break;
        }

        int    ok;
        double v = evaluate(expr, &ok);

        if (!ok) {
            strcpy(result, "Error");
            printf("  [Error] 表达式无效: 括号不匹配 / 非法字符 / 除数为 0\n");
        } else {
            snprintf(result, sizeof(result), "%.6g", v);
        }
    }

    return 0;
}
