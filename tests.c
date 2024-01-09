#include "error.h"
#include "lex.h"
#include "list.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <vadefs.h>

#define assert_fpos(line,col) {\
    if (lineno != line || columnno != col) {\
        ERROR("File position should be (line %u, col %u), but actually got (line %u, col %u)",\
                line, col, lineno, columnno);\
    }\
}

void token_test(char *s, Token tkn) {

    Token t = lex_token(s, 0, 0);
    printf("%s\n", format_token(t));
    if(!token_eq(t, tkn)){
        ERROR("For input string '%s', expected\n%s\nbut got:\n%s", s, format_token(tkn), format_token(t));
    }
    if (!token_eq(lex_token(token2str(tkn), 0, 0), tkn)) {
        ERROR("Token from input string '%s', detokenized to '%s'", s, token2str(t));
    }
    free_tkn(t);
}

#define list_test(s, l, ...) _list_test(__LINE__, s, l, __VA_ARGS__);

void _list_test(size_t line, size_t size, List l, ...){

    if (l.size != size) {
        fprintf(stderr, "On Line %zu: ", line);
        ERROR("List should be size %zu, but was actually %zu", size, l.size);
    }
    va_list args;
    va_start(args, l);
    
    Token t = {0};
    for (size_t i = 0; i < l.size; i++){
        if (lget(l, i, &t)){
            fprintf(stderr, "On Line %zu: ", line);
            ERROR("List out of bounds at %zu for list of size %zu", i, l.size);
        }

        int exp = va_arg(args, int);
        if (t.value.i != exp){
            fprintf(stderr, "On Line %zu: ", line);
            ERROR("List element %zu should be %d, but found %d", i, exp, t.value.i);
        }
    }

    va_end(args);
}

int main(void){

    printf("HI\n");
    //////////// Obfuscated emoticons ////////////
    for (int i = 0; i < (int)sizeof(OBFUSCATED_CHARS); i++){
        if (deobfuscate_emoticon(OBFUSCATED_EMO[i]) != OBFUSCATED_CHARS[i]){
            ERROR("Assertion error: '%s' should be deobfuscated to '%c' not '%c'", OBFUSCATED_EMO[i], OBFUSCATED_CHARS[i], deobfuscate_emoticon(OBFUSCATED_EMO[i]));
        }
    }

    ////////// Tokenizing /////////
    FILE *codefile = fopen("test.txt", "r");
    unsigned int lineno = 0;
    unsigned int columnno = 0;

    skip_ws(codefile, &lineno, &columnno);
    assert_fpos(0, 0);
    char *c = get_lexme(codefile, &columnno);
    assert(strcmp(c, "abc") == 0);
    assert_fpos(0, 3);
    free(c);
    c = NULL;
    skip_ws(codefile, &lineno, &columnno);
    assert_fpos(1, 0);
    c = get_lexme(codefile, &columnno);
    assert(strcmp(c, "def") == 0);
    assert_fpos(1, 3);

    fclose(codefile);

    ///////// Lexing //////////
    token_test(":)`", (Token){
        .line = 0,
        .column = 0,
        .type = OBFUS,
        .value.str = "A"
    });

    token_test("abc", (Token){
        .line = 0,
        .column = 0,
        .type = STR,
        .value.str = "abc"
    });

    token_test(":-O", (Token){
        .line = 0,
        .column = 0,
        .type = EMOTICON,
        .value.emoticon = (Emoticon){
            .eyes = ":",
            .nose = '-',
            .op = SET_CURRENT
        }
    });

    token_test("^_^", (Token){
        .line = 0,
        .column = 0,
        .type = EMOTICON,
        .value.emoticon = (Emoticon){
            .eyes = NULL,
            .nose = '\0',
            .op = OBFUSCATION_ON
        }
    });

    token_test("^__^", (Token){
        .line = 0,
        .column = 0,
        .type = EMOTICON,
        .value.emoticon = (Emoticon){
            .eyes = NULL,
            .nose = '\0',
            .op = OBFUSCATION_OFF
        }
    });

    token_test(":::-O", (Token){
        .line = 0,
        .column = 0,
        .type = EMOTICON,
        .value.emoticon = (Emoticon){
            .eyes = ":::",
            .nose = '-',
            .op = SET_CURRENT
        }
    });

    token_test(":-^", (Token){
        .line = 0,
        .column = 0,
        .type = STR,
        .value.str = ":-^"
    });

    token_test(":-", (Token){
        .line = 0,
        .column = 0,
        .type = STR,
        .value.str = ":-"
    });

    token_test(":O", (Token){
        .line = 0,
        .column = 0,
        .type = EMOTICON,
        .value.emoticon = (Emoticon) {
            .eyes = ":",
            .nose = '\0',
            .op = SET_CURRENT
        }
    });

    token_test("1", (Token){
        .line = 0,
        .column = 0,
        .type = INT,
        .value.i = 1
    });

    token_test("-1", (Token){
        .line = 0,
        .column = 0,
        .type = INT,
        .value.i = -1
    });

    token_test("123456789", (Token){
        .line = 0,
        .column = 0,
        .type = INT,
        .value.i = 123456789
    });

    token_test("-123456789", (Token){
        .line = 0,
        .column = 0,
        .type = INT,
        .value.i = -123456789
    });

    token_test("1.1", (Token){
        .line = 0,
        .column = 0,
        .type = DOUBLE,
        .value.d = 1.1
    });

    token_test("-1.1", (Token){
        .line = 0,
        .column = 0,
        .type = DOUBLE,
        .value.d = -1.1
    });

    /// List ///

    List l = create_list(NULL, 1);

    // Insert
    assert(!linsert(&l, 0, lex_token("0", 0, 0)));
    assert(!linsert(&l, 0, lex_token("1", 0, 0)));
    assert(!linsert(&l, 2, lex_token("2", 0, 0)));
    assert(!linsert(&l, 1, lex_token("3", 0, 0)));

    for (size_t i = 0; i < l.size; i++){
        printf("%s\n", format_token(l.arr[i]));
    }

    list_test(4, l, 1, 3, 0, 2);

    // Rotate
    lrotate(&l, 2);
    list_test(4, l, 0, 2, 1, 3);
    lrotate(&l, -2);
    list_test(4, l, 1, 3, 0, 2);

    // Insert when rotated
    lrotate(&l, 2);
    list_test(4, l, 0, 2, 1, 3);
    assert(!linsert(&l, 0, lex_token("4", 0, 0)));
    assert(!linsert(&l, 5, lex_token("5", 0, 0)));
    list_test(6, l, 4, 0, 2, 1, 3, 5);

    // Remove
    lrotate(&l, -2);
    assert(!lremove(&l, 5));
    list_test(5, l, 2, 1, 3, 5, 4);
    assert(!lremove(&l, 0));
    list_test(4, l, 1, 3, 5, 4);
    assert(!lremove(&l, 2));
    list_test(3, l, 1, 3, 4);

    // Remove when rotated
    assert(!linsert(&l, 0, lex_token("7", 0, 0)));
    assert(!linsert(&l, 0, lex_token("8", 0, 0)));
    assert(!linsert(&l, 0, lex_token("9", 0, 0)));
    lrotate(&l, 3);
    list_test(6, l, 1, 3, 4, 9, 8, 7);

    assert(!lremove(&l, 0));
    list_test(5, l, 3, 4, 9, 8, 7);
    assert(!lremove(&l, 4));
    list_test(4, l, 3, 4, 9, 8);

    // Reverse

    lreverse(&l);
    list_test(4, l, 8, 9, 4, 3);
    lreverse(&l);
    list_test(4, l, 3, 4, 9, 8);
    assert(!linsert(&l, 0, lex_token("10", 0, 0)));
    list_test(5, l, 10, 3, 4, 9, 8);
    lreverse(&l);
    list_test(5, l, 8, 9, 4, 3, 10);

    lfree(l);

    return 0;
}