#include <cstring>
#include <cassert>
#include <cstdio>
#include <cwchar>
#include <clocale>

bool IS_DBG_MODE = false;
#define dbgmsg(...) if (IS_DBG_MODE) printf(__VA_ARGS__)

#include "test.h"
#include "lang.cc"

int main()
{
    setlocale(LC_CTYPE, "");
    char buf[16];
    LangPrimitive prim;

    test_case("c3 converts to midi value for middle c", {
        strcpy(buf, "c3");
        prim = lang_parse_note(buf);
        test(prim.val).eq(60);
    });

    test_case("c-2 is the lowest midi value available", {
        strcpy(buf, "c-2");
        prim = lang_parse_note(buf);
        test(prim.val).eq(0);
    });

    test_case("flat modifier cannot work for c-2", {
        strcpy(buf, "c-2b");
        prim = lang_parse_note(buf);
        test((int)prim.type).eq(0);
        test((int)prim.val).eq(-1);
    });

    test_case("there can be no lower octave that -2", {
        strcpy(buf, "b-3");
        prim = lang_parse_note(buf);
        test((int)prim.type).eq(0);
        test((int)prim.val).eq(-1);
    });

    test_case("note a-2 has value of 9", {
        strcpy(buf, "a-2");
        prim = lang_parse_note(buf);
        test((int)prim.val).eq(9);
    });

    test_case("g8 has max midi value of 127", {
        strcpy(buf, "g8");
        prim = lang_parse_note(buf);
        test((int)prim.type).eq(1);
        test((int)prim.val).eq(127);
    });

    test_case("note value cannot exceed g8", {
        strcpy(buf, "g8#");
        prim = lang_parse_note(buf);
        test((int)prim.type).eq(0);
        test((int)prim.val).eq(128);
    });

    test_case("note value is terminated by whitespace", {
        strcpy(buf, "c3 ");
        prim = lang_parse_note(buf);
        test((int)prim.type).eq(1);
        test((int)prim.val).eq(60);

        strcpy(buf, "c3\t");
        prim = lang_parse_note(buf);
        test((int)prim.type).eq(1);
        test((int)prim.val).eq(60);

        strcpy(buf, "c3\n");
        prim = lang_parse_note(buf);
        test((int)prim.type).eq(1);
        test((int)prim.val).eq(60);
    });
    printf("\n\nTests completed :)\n\n");
    return 0;
}
