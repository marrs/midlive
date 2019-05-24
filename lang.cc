enum struct LangCharType {  // The actual char that can appear in buffer
    Error           = 0x000,
    SemitoneMod     = 0x001,
    NoteName        = 0x002,
    Letter          = 0x004,
    Digit           = 0x010,
    Minus           = 0x020,
    Symbol          = 0x040,
    Quote           = 0x080,
    ListBegin       = 0x100,
    ListEnd         = 0x200,
    BlockBegin      = 0x400,
    BlockEnd        = 0x800,
};

enum struct LangTokenType {
    Error           = 0x000,
    Note            = 0x001,
};

struct LangPrimitive {
    enum LangTokenType type;
    int val;
};

int lang_char_type(char &x) {
    int types = 0x000;
    switch (x) {
        case 'b': return ((int)LangCharType::Letter | (int)LangCharType::NoteName | (int)LangCharType::SemitoneMod);
        case 'a':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g': return (int)LangCharType::Letter | (int)LangCharType::NoteName;
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z': return (int)LangCharType::Letter;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6': 
        case '7':
        case '8':
        case '9': return (int)LangCharType::Digit;
        case '-': return (int)LangCharType::Symbol | (int)LangCharType::Minus;
        case '#': return (int)LangCharType::Symbol | (int)LangCharType::SemitoneMod;
        case '"': return (int)LangCharType::Symbol | (int)LangCharType::Quote;
        case '(': return (int)LangCharType::Symbol | (int)LangCharType::ListBegin;
        case ')': return (int)LangCharType::Symbol | (int)LangCharType::ListEnd;
        case '{': return (int)LangCharType::Symbol | (int)LangCharType::BlockBegin;
        case '}': return (int)LangCharType::Symbol | (int)LangCharType::BlockEnd;
        case '.': return (int)LangCharType::Symbol;
    }
    return (int)LangCharType::Error;
}

bool is_char_type(LangCharType type, char ch)
{
    int charType = lang_char_type(ch);
    return (int)charType == ((int)type | charType);
}

int note_to_midi(char ch)
{
    assert(is_char_type(LangCharType::NoteName, ch));
    switch (ch) {
        case 'c': return 24;
        case 'd': return 26;
        case 'e': return 28;
        case 'f': return 29;
        case 'g': return 31;
        case 'a': return 33;
        case 'b': return 35;
        default: return -1;
    }
}

int to_int(char x)
{
    switch (x) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return -1;
    }
}

struct LangToken {
    enum LangCharType type;
    LangToken *next; // Next char type to expect (null means you've finished).
    LangToken *alt; // Alternative to try if this type does not match
};

LangPrimitive lang_parse_note(char *buf)
{
    if (!is_char_type(LangCharType::Letter, *buf)) {
        return { LangTokenType::Error, -1};
    }

    LangToken note[8] = {
        { LangCharType::NoteName,    &note[1],  NULL     }, // 0
        { LangCharType::Minus,       &note[2],  &note[2] }, // 1
        { LangCharType::Digit,       &note[3],  NULL     }, // 2
        { LangCharType::SemitoneMod, NULL,      NULL     }, // 3
    };

    LangToken *rulePos = note;
    int val = -1;
    int octave = 0;
    int octaveSign = 1;
    char *bufptr = buf;
    while (*bufptr != '\0'
    &&     *bufptr != '\t'
    &&     *bufptr != '\n'
    &&     *bufptr != ' '
    ){
        char ch = *bufptr;
        dbgmsg("'%c'", ch);
        if (is_char_type(rulePos->type, ch)) {
            dbgmsg(" matches char type %d\n", rulePos->type);
            switch (rulePos->type) {
                case LangCharType::NoteName: {
                    val = note_to_midi(ch);
                } break;
                case LangCharType::Minus: {
                    octaveSign = -1;
                } break;
                case LangCharType::Digit: {
                    octave = to_int(ch) * 12;
                } break;
                case LangCharType::SemitoneMod: {
                    val += ch == '#'? 1 : -1;
                } break;
                default: break;
            }
            if (NULL != rulePos->next) {
                rulePos = rulePos->next;
                ++bufptr;
                continue;
            } else if (NULL != rulePos->alt) {
                rulePos = rulePos->alt;
                continue;
            } else {
                break;
            }
        } else {
            dbgmsg(" does not match char type %d.\n", rulePos->type);
            if (NULL == rulePos->alt) {
                return { LangTokenType::Error, -1 };
            } else {
                rulePos = rulePos->alt;
                continue;
            }
        }
    }
    octave *= octaveSign;
    val += octave;
    if (-1 < val && val < 128) {
        return { LangTokenType::Note, val };
    }
    return { LangTokenType::Error, val };
}
