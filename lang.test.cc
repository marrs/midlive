#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cwchar>
#include <clocale>

bool IS_DBG_MODE = false;
#define dbgmsg(...) if (IS_DBG_MODE) printf(__VA_ARGS__)

// Abstract call to malloc so that we can implement our own
// memory manager in the future if desired.
#define mem(TYPE, SIZE) (TYPE *)malloc(SIZE * sizeof(TYPE))

#include "test.h"
#include "lang.cc"

int main()
{
    setlocale(LC_CTYPE, "");
    char buf[16];
    LangPrimitive prim;

    test_case("c3 converts to midi value for middle c", {
        strcpy(buf, "c3");
        prim = lang_parse_pitch(buf);
        test(is_only_pitch(prim));
        test(prim.pitch).eq(60);
    });

    test_case("c-2 is the lowest midi value available", {
        strcpy(buf, "c-2");
        prim = lang_parse_pitch(buf);
        test(is_only_pitch(prim));
        test(prim.pitch).eq(0);
    });

    test_case("flat modifier cannot work for c-2", {
        strcpy(buf, "c-2b");
        prim = lang_parse_pitch(buf);
        test((int)prim.error).eq((int)LangErrorType::OutOfBoundsPitch);
        test(is_only_pitch(prim));
        test(prim.pitch).eq(-1);
    });

    test_case("there can be no lower octave that -2", {
        strcpy(buf, "b-3");
        prim = lang_parse_pitch(buf);
        test((int)prim.error).eq((int)LangErrorType::OutOfBoundsPitch);
        test(is_only_pitch(prim));
        test((int)prim.pitch).eq(-1);
    });

    test_case("note a-2 has value of 9", {
        strcpy(buf, "a-2");
        prim = lang_parse_pitch(buf);
        test(is_only_pitch(prim));
        test(prim.pitch).eq(9);
    });

    test_case("g8 has max midi value of 127", {
        strcpy(buf, "g8");
        prim = lang_parse_pitch(buf);
        test(is_only_pitch(prim));
        test(prim.pitch).eq(127);
    });

    test_case("note value cannot exceed g8", {
        strcpy(buf, "g8#");
        prim = lang_parse_pitch(buf);
        test(is_only_pitch(prim));
        test((int)prim.error).eq((int)LangErrorType::OutOfBoundsPitch);
        test(prim.pitch).eq(128);
    });

    test_case("note value is terminated by whitespace", {
        strcpy(buf, "c3 ");
        prim = lang_parse_pitch(buf);
        test(is_only_pitch(prim));
        test(prim.pitch).eq(60);

        strcpy(buf, "c3\t");
        prim = lang_parse_pitch(buf);
        test(is_only_pitch(prim));
        test(prim.pitch).eq(60);

        strcpy(buf, "c3\n");
        prim = lang_parse_pitch(buf);
        test(is_only_pitch(prim));
        test(prim.pitch).eq(60);
    });

    test_case("normal length value", {
        strcpy(buf, "4");
        prim = lang_parse_length(buf);
        test(is_only_length(prim));
        test(prim.lengthMod).eq(LengthMod::Normal);
        test(prim.length).eq(4);
    })

    test_case("dotted length value", {
        strcpy(buf, "4d");
        prim = lang_parse_length(buf);
        test(is_only_length(prim));
        test(prim.lengthMod).eq(LengthMod::Dotted);
    })

    test_case("triplet length value", {
        strcpy(buf, "4t");
        prim = lang_parse_length(buf);
        test(is_only_length(prim));
        test(prim.lengthMod).eq(LengthMod::Triplet);
    })

    test_case("note of unmodified length", {
        strcpy(buf, "8.c3");
        prim = lang_parse_note(buf);
    })

    test_case("note of modified length", {
        // This example demonstrates the need for the dot by
        // exposing the ambiguity of the 'd'. Does it mean
        // "dotted" or "rae"?
        strcpy(buf, "4d.d4");
        prim = lang_parse_note(buf);
    })
    
    test_case("interval of length", {
        // In this case the dot prefix denoting the interval
        // is shared with the dot suffix denoting the length.
        strcpy(buf, "8.5");
        prim = lang_parse_note(buf);
    });

    test_case("note of length must be denoted with the length first", {
        // This is to leave us the option of defining chords
        // with intervals.
        prim = lang_parse_note(buf);
        strcpy(buf, "c3.8");
    });

    test_case("list of pitches", {
        strcpy(buf, "(c3 d4 e5)");
        LangList list;
        setup(list, 3);
        lang_parse_list(buf, list);
        test(is_only_pitch(list.primitives[0]));
        test((int)list.primitives[0].pitch).eq(60);

        test(is_only_pitch(list.primitives[1]));
        test((int)list.primitives[1].pitch).eq(74);

        test(is_only_pitch(list.primitives[2]));
        test((int)list.primitives[2].pitch).eq(88);
    });

    printf("\n\n\tTests completed :)\n\n");
    return 0;
}
