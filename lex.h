#ifndef __LEX_H__
#define __LEX_H__

#include <stdio.h>
#include <stdbool.h>

extern FILE* codefile;

static const char OBFUSCATED_CHARS[] = "ABCDEFGHIJKLMNOPQSTUVWXYZ0123456789";

static const char OBFUSCATED_EMO[36][4] = {
    ":)`", "8)`", "B)`", ":]`", "8]`", "B]`", ":[`", "8[`", "B[`", ":(`", "8(`", "B(`",
    ":)-", "8)-", "B)-", ":]-", "8]-", "B]-", ":[-", "8[-", "B[-", ":(-", "8(-", "B(-",
    ":)~", "8)~", "B)~", ":]~", "8]~", "B]~", ":[~", "8[~", "B[~", ":(~", "8(~", "B(~",
};

typedef enum {
    SET_CURRENT = 'O',
    COUNT = 'C',
    REVERSE = 'X',
    ROTATE = '@',
    MOVE_LEFT = '<',
    MOVE_RIGHT = '>',
    COPY_LEFT = '[',
    COPY_RIGHT = ']',
    ASSIGN = 'D',
    INSERT = 'V',
    EXPLODE_LEFT = '7',
    EXPLODE_RIGHT = 'L',
    IMPLODE_LEFT = '#',
    IMPLODE_RIGHT = '$',
    PRINT = 'P',
    PRINT_AND_POP = 'Q',
    INPUT = '*',
    MATHS_LEFT = '{',
    MATHS_RIGHT = '}',
    COMPARE_LEFT = '\\',
    COMPARE_RIGHT = '/',
    OPEN_BLOCK = '(',
    CLOSE_BLOCK = ')',
    DIVIDE_BLOCK = '|',
    BREAK = '3',
    BREAK_AND_POP = 'E',
    OBFUSCATION_ON = 1,
    OBFUSCATION_OFF = 2,
} Op_Type;

static const Op_Type mouth_optype_table[256] = {
    ['O'] = SET_CURRENT,
    ['C'] = COUNT,
    ['X'] = REVERSE,
    ['@'] = ROTATE,
    ['<'] = MOVE_LEFT,
    ['>'] = MOVE_RIGHT,
    ['['] = COPY_LEFT,
    [']'] = COPY_RIGHT,
    ['D'] = ASSIGN,
    ['V'] = INSERT,
    ['7'] = EXPLODE_LEFT,
    ['L'] = EXPLODE_RIGHT,
    ['#'] = IMPLODE_LEFT,
    ['$'] = IMPLODE_RIGHT,
    ['P'] = PRINT,
    ['Q'] = PRINT_AND_POP,
    ['*'] = INPUT,
    ['{'] = MATHS_LEFT,
    ['}'] = MATHS_RIGHT,
    ['\\'] = COMPARE_LEFT,
    ['/'] = COMPARE_RIGHT,
    ['('] = OPEN_BLOCK,
    [')'] = CLOSE_BLOCK,
    ['|'] = DIVIDE_BLOCK,
    ['3'] = BREAK,
    ['E'] = BREAK_AND_POP,
};

typedef struct {
    Op_Type op;
    char nose;
    char* eyes;
} Emoticon;

typedef struct {
    enum {
        STR,
        EMOTICON,
        OBFUS,
        INT,
        DOUBLE
    } type;
    union {
        char* str;
        Emoticon emoticon;
        int i;
        double d;
    } value;
    unsigned int line;
    unsigned int column;
} Token;

char deobfuscate_emoticon(const char face[3]);

void skip_ws(FILE *f, unsigned int *line, unsigned int *col);
char *get_lexme(FILE *f, unsigned int *col);
Token lex_token(const char *s, unsigned int lineno, unsigned int columnno);

void free_tkn(Token t);
bool token_eq(Token a, Token b);
Token copy_tkn(const Token t);
char *token2str(Token t);
char *format_token(Token t);

#endif