#include "lex.h"
#include "error.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <stdarg.h>

/* Error */

void lex_err(unsigned int lineno, unsigned int columnno, const char* msg, ...) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, " (line %u, col %u)\n", lineno, columnno);
    va_end(args);
    exit(1);
}

/* Tokenizing */

char deobfuscate_emoticon(const char face[3]){
    int obs_index;

    switch(face[2]){
        case '`':
            obs_index = 0;
            break;
        case '-':
            obs_index = 12;
            break;
        case '~':
            obs_index = 24;
            break;
        default:
            // ERROR("Lexing Error: Obfusticated emoticon '%s' cannot have beard '%c'", 
            //     face, face[2]);
            // break;
            return '\0';
    }

    switch(face[1]){
        case ')':
            obs_index += 0;
            break;
        case ']':
            obs_index += 3;
            break;
        case '[':
            obs_index += 6;
            break;
        case '(':
            obs_index += 9;
            break;
        default:
            // ERROR("Lexing Error: Obfusticated emoticon '%s' cannot have mouth '%c'", 
            //     face, face[1]);
            // break;
            return '\0';
    }

    switch(face[0]){
        case ':':
            obs_index += 0;
            break;
        case '8':
            obs_index += 1;
            break;
        case 'B':
            obs_index += 2;
            break;
        default:
            // ERROR("Lexing Error: Obfusticated emoticon '%s' cannot have eyes '%c'", 
            //     face, face[0]);
            // break;
            return '\0';
    }

    if (obs_index < 0 || obs_index > (int)sizeof(OBFUSCATED_CHARS)){
        ERROR("Internal Error: Face '%s' resulted in obfuscated character of index %d "
                "(should be less that %zu)", face, obs_index, sizeof(OBFUSCATED_CHARS));
    }

    return OBFUSCATED_CHARS[obs_index];
}

Token lex_token(const char *s, unsigned int lineno, unsigned int columnno){
    Token t = (Token){
        .column = columnno,
        .line = lineno,
    };

    size_t tkn_len = strlen(s);

    // May be obfuscated char
    if (tkn_len == 3) {
        char dbf = deobfuscate_emoticon(s);
        if (dbf) {
            t.type = OBFUS;
            t.value.str = (char *)calloc(2, sizeof(char));
            t.value.str[0] = dbf;
            return t;
        }
    }

    // May be obfustication switch
    if (!strcmp("^_^", s) || !strcmp("^__^", s)){
        t.type = EMOTICON;
        t.value.emoticon = (Emoticon){
            .eyes = NULL,
            .nose = '\0',
            // ^_^ = on, ^__^ = off
            .op = tkn_len == 3 ? OBFUSCATION_ON : OBFUSCATION_OFF
        };
        return t;
    }

    // May be other operation
    if (tkn_len >= 2 && mouth_optype_table[(int) s[tkn_len - 1]]) {
        t.type = EMOTICON;

        // Note: 2 character emoticons are valid: eyes are first character and mouth is second
        t.value.emoticon = (Emoticon){
            .op = (Op_Type) s[tkn_len - 1], // Enum values are the same as the mouth characters
            .nose = tkn_len > 2 ? s[tkn_len - 2] : '\0',
            .eyes = strndup(s, tkn_len > 2 ? tkn_len - 2 : 1)
        };

        // Terminate reused string early so eyes don't include nose/mouth
        return t;
    }

    // May be a int

    char *end = NULL;
    long ival = strtol(s, &end, 10);
    if (end == s + strlen(s)){
        if (ival > INT_MAX || ival < INT_MIN){
            lex_err(lineno, columnno, "Integer value %ld overflow bounds [%d, %d]", ival, INT_MIN, INT_MAX);
        }
        t.type = INT;
        t.value.i = (int) ival;
        return t;
    }

    // May be a double
    end = NULL;
    double dval = strtod(s, &end);
    if (end == s + strlen(s)){
        t.type = DOUBLE;
        t.value.d = dval;
        return t;
    }

    // Otherwise, treat as a string
    t.type = STR;
    t.value.str = strdup(s);
    return t;
}

void free_tkn(Token t) {
    if ((t.type == OBFUS || t.type == STR) && t.value.str) {
        free(t.value.str);
        t.value.str = NULL;
    } else if (t.type == EMOTICON && t.value.emoticon.eyes) {
        free(t.value.emoticon.eyes);
        t.value.emoticon.eyes = NULL;
    } else if (t.type > DOUBLE) {
        ERROR("Token type is invalid value '%d'", t.type);
    }
}

Token copy_tkn(const Token t){
    Token newt = (Token){
        .column = t.column,
        .line = t.line,
        .type = t.type,
        .value = t.value
    };
    switch (t.type) {
        case OBFUS:
        case STR:
            newt.value.str = strdup(t.value.str);
            return newt;
        case EMOTICON:
            newt.value.emoticon.eyes = strdup(t.value.emoticon.eyes);
            return newt;
        default:
            return newt;
    }
}

bool token_eq(const Token a, const Token b) {
    if (a.type != b.type || a.line != b.line || a.column != b.column) {
        return false;
    }

    if (a.type == OBFUS || a.type == STR) {
        return !strcmp(a.value.str, b.value.str);
    } else if (a.type == INT) {
        return a.value.i == b.value.i;
    } else if (a.type == DOUBLE) {
        return a.value.d == b.value.d;
    } else if (a.type == EMOTICON) {
        Emoticon ae = a.value.emoticon;
        Emoticon be = a.value.emoticon;

        if (ae.eyes && be.eyes && strcmp(ae.eyes, be.eyes)){
            return false;
        } else if ((!ae.eyes || !be.eyes) && ae.eyes != be.eyes) {
            return false;
        }

        return ae.nose == be.nose && ae.op == be.op;
    } else {
        ERROR("Token type is invalid value '%d'", a.type);
        return false;
    }
}

char *token2str(const Token t) {
    switch (t.type) {
        case STR:
            return strdup(t.value.str);
        case OBFUS:
            for (int i = 0; i < (int) sizeof(OBFUSCATED_CHARS) - 1; i++) {
                if (OBFUSCATED_CHARS[i] == t.value.str[0]) {
                    return strdup(OBFUSCATED_EMO[i]);
                }
            }
            ERROR("Tried to detokenize obfuscated emoticon with invalid value '%s'", t.value.str);
        case EMOTICON: {
            Emoticon e = t.value.emoticon;
            if (e.op == OBFUSCATION_OFF || e.op == OBFUSCATION_ON) {
                return strdup(e.op == OBFUSCATION_OFF ? "^__^" : "^_^");
            }
            char *res = (char *)calloc(1000, sizeof(char));
            if (e.nose) {
                sprintf(res, "%s%c%c", e.eyes, e.nose, e.op);
            } else {
                sprintf(res, "%s%c", e.eyes, e.op);
            }
            return res;
        }
        case DOUBLE: {
            size_t len = snprintf(NULL, 0, "%f", t.value.d);
            char *res = (char *)calloc(len + 1, sizeof(char));
            snprintf(res, len + 1, "%f", t.value.d);
            return res;
        }
        case INT: {
            size_t len = snprintf(NULL, 0, "%d", t.value.i);
            char *res = (char *)calloc(len + 1, sizeof(char));
            snprintf(res, len + 1, "%d", t.value.i);
            return res;
        }
        default:
            ERROR("Bad token type %d", t.type);
    }
}

/**
 * @brief Reverts a token back into
 * 
 * @param t 
 * @return char* 
 */
char *format_token(const Token t) {
    char *tstr = t.type == OBFUS    ? "OBFUS"    :
                 t.type == STR      ? "STR"      :
                 t.type == EMOTICON ? "EMOTICON" :
                 t.type == INT      ? "INT"      :
                 t.type == DOUBLE   ? "DOUBLE"   :
                                      "???"      ;
    
    char *buf = (char *)calloc(1000, sizeof(char));
    char *tokstr = token2str(t);
    sprintf(buf, "Token{\n\t.line = %u,\n\t.column = %u,\n\t.type = %s\n\t.value = %s\n}", t.line, t.column, tstr, tokstr);
    free(tokstr);
    return buf;
}

/* Lexing the file */

/**
 * @brief Skips white space in file.
 * @details the character after the whitespace, so getch will be that character.
 * This also means any ungot character will be overwritten.
 * 
 */
void skip_ws(FILE *f, unsigned int *line, unsigned int *col) {
    int c;
    while (isspace(c = getc(f)) && !feof(f)){
        if (c == '\n'){
            *col = 0;
            *line += 1;
        } else {
            *col += 1;
        }
    }
    ungetc(c, f);
}

/**
 * @brief Get the full string tokenized
 * 
 * @return Full character string
 * @throws Exits with an assertion error if data cannot be (re)allocated
 */
char *get_lexme(FILE *f, unsigned int *col){
    char *token = (char *) calloc(51, sizeof(char));
    assert(token);
    size_t curr_size = 0;
    size_t max_size = 51;

    int c;
    while (!isspace((c = getc(f))) && !feof(f)) {
        token[curr_size++] = c;
        *col += 1;
        if (curr_size >= max_size) {
            char *temp = (char *) realloc(token, max_size * 2);
            assert(temp);
            token = temp;
            max_size *= 2;
        }
    }

    ungetc(c, f);
    token[curr_size] = '\0';
    return token;
}

