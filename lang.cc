enum struct LangCharType {  // The actual char that can appear in buffer
    Error           =  0x000,
    SemitoneMod     =  0x001,
    NoteName        =  0x002,
    Letter          =  0x004,
    Digit           =  0x010,
    Minus           =  0x020,
    Symbol          =  0x040,
    Quote           =  0x080,
    ListBegin       =  0x100,
    ListEnd         =  0x200,
    BlockBegin      =  0x400,
    BlockEnd        =  0x800,
};

enum struct LangCharLenType {
    Error          = 0x000,
    Pos1           = 0x001,
    Pos2           = 0x002,
    LengthMod      = 0x004,
};

enum struct LangTokenType {
    Error           = 0x000,
    Pitch           = 0x001,
    LengthNormal    = 0x002,
    LengthDotted    = 0x004,
    LengthTriplet   = 0x008,
    Note            = 0x010,
};

struct LangPrimitive {
    enum LangTokenType type;
    int val;
};

int lang_char_len_pos(char &x) {
    switch (x) {
        case '1':
        case '3':
        case '8': return (int)LangCharLenType::Pos1;
        case '2':
        case '4':
        case '6': return (int)LangCharLenType::Pos1 | (int)LangCharLenType::Pos2;
        case 'd': return (int)LangCharLenType::LengthMod;
        case 't': return (int)LangCharLenType::LengthMod;
        default:  return (int)LangCharLenType::Error;
    }
}

int lang_char_type(char &x) {
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

bool is_char_type(LangCharLenType type, char ch)
{
    int charType = lang_char_len_pos(ch);
    return (int)charType == ((int)type | charType);
}

int pitch_to_midi(char ch)
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

struct LangToken {
    enum LangCharType type;
    LangToken *next; // Next char type to expect (null means you've finished).
    LangToken *alt; // Alternative to try if this type does not match
};

struct LangLenToken {
    enum LangCharLenType type;
    LangLenToken *next; // Next char type to expect (null means you've finished).
    LangLenToken *alt; // Alternative to try if this type does not match
};

#define is_whitespace_char(CHR) \
    ((CHR) == '\0'   \
||  (CHR) == '\t'   \
||  (CHR) == '\n'   \
||  (CHR) == ' ')

#define lang_abstract_parse_char(SWITCH_CASES) \
        dbgmsg("Character '%c'", ch); \
        if (is_char_type(rulePos->type, ch)) { \
            dbgmsg(" matches char type %d\n", rulePos->type); \
            switch (rulePos->type) SWITCH_CASES \
            if (NULL != rulePos->next) { \
                rulePos = rulePos->next; \
                ++*buf; \
                continue; \
            } else if (NULL != rulePos->alt) { \
                rulePos = rulePos->alt; \
                continue; \
            } else { \
                break; \
            } \
        } else { \
            dbgmsg(" does not match char type %d.\n", rulePos->type); \
            if (NULL == rulePos->alt) { \
                return { LangTokenType::Error, -1 }; \
            } else { \
                rulePos = rulePos->alt; \
                continue; \
            } \
        }

LangPrimitive lang_parse_length(char **buf)
{
    if (!is_char_type(LangCharType::Digit, **buf)) {
lang_parse_length_default:
        return { LangTokenType::Error, -1};
    }

    LangLenToken parseRules[3] = {
        { LangCharLenType::Pos1,      &parseRules[1], NULL           }, // 0
        { LangCharLenType::Pos2,      NULL,           &parseRules[2] }, // 1
        { LangCharLenType::LengthMod, NULL,           NULL           }, // 2
    };

    LangLenToken *rulePos = parseRules;
    char lenStr[2];
    int lenVal = -1;
    LangTokenType type = LangTokenType::LengthNormal;

    while(!is_whitespace_char(**buf)) {
        char ch = **buf;
        char *lenValPos = lenStr;
        lang_abstract_parse_char({
            case LangCharLenType::Pos1:
            case LangCharLenType::Pos2: {
                *lenValPos++ = ch;
            } break;
            case LangCharLenType::LengthMod: {
                if ('d' == ch) {
                    type = LangTokenType::LengthDotted;
                } else if ('t' == ch) {
                    type = LangTokenType::LengthTriplet;
                }
            } break;
            default: { goto lang_parse_length_default; }
        });
    }
    lenVal = atoi(lenStr);
    return { type, lenVal };
}

inline LangPrimitive lang_parse_length(char *buf)
{
    char **bufptr = &buf;
    return lang_parse_length(bufptr);
}

inline bool lang_last_char_check(char *chptr, char *lastChar)
{
    return nullptr == lastChar? true : chptr != lastChar;
}

LangPrimitive lang_parse_pitch(char **buf, char *lastChar = nullptr)
{
    if (!is_char_type(LangCharType::Letter, **buf)) {
        return { LangTokenType::Error, -1};
    }

    LangToken parseRules[4] = {
        { LangCharType::NoteName,    &parseRules[1], NULL     }, // 0
        { LangCharType::Minus,       &parseRules[2], &parseRules[2] }, // 1
        { LangCharType::Digit,       &parseRules[3], NULL     }, // 2
        { LangCharType::SemitoneMod, NULL,           NULL     }, // 3
    };

    LangToken *rulePos = parseRules;
    int val = -1;
    int octave = 0;
    int octaveSign = 1;


    bool lastCharCheck = nullptr != lastChar? *buf != lastChar : true;
    while(!is_whitespace_char(**buf) && lang_last_char_check(*buf, lastChar)) {
        char ch = **buf;
        lang_abstract_parse_char({
            case LangCharType::NoteName: {
                val = pitch_to_midi(ch);
            } break;
            case LangCharType::Minus: {
                octaveSign = -1;
            } break;
            case LangCharType::Digit: {
                octave = atoi(&ch) * 12;
            } break;
            case LangCharType::SemitoneMod: {
                val += ch == '#'? 1 : -1;
            } break;
            default: break;
        });
    }
    octave *= octaveSign;
    val += octave;
    if (-1 < val && val < 128) {
        return { LangTokenType::Pitch, val };
    }
    return { LangTokenType::Error, val };
}

inline LangPrimitive lang_parse_pitch(char *buf)
{
    char **bufptr = &buf;
    return lang_parse_pitch(bufptr);
}

LangPrimitive lang_parse_note(char **buf, char *lastChar = nullptr)
{
    // TODO: Refactor parse rules for pitch and length
    // so that we can reuse them for construction of
    // parse rules for note (which is basically length
    // followed by noted.  Parse rules should probably
    // go in their own module or struct.
}

inline LangPrimitive lang_parse_note(char *buf)
{
    char **bufptr = &buf;
    return lang_parse_note(bufptr);
}

struct LangList {
    LangPrimitive *primitives;
    size_t len;
    LangPrimitive *ptr;
};

void setup(LangList &list, size_t len)
{
    list.primitives = mem(LangPrimitive, len);
    list.len = len;
    list.ptr = list.primitives;
}

void lang_parse_list(char *buf, LangList &list)
{
    if (!is_char_type(LangCharType::ListBegin, *buf)) {
        return; // TODO?: Return LangList
    }
    ++buf;
    char *lastChar = buf;

    for (; !is_char_type(LangCharType::ListEnd, *lastChar); ++lastChar) {}
    while (true) {
        char **bufptr = &buf;
        if (is_char_type(LangCharType::Letter, *buf)) {
            LangPrimitive primitive = lang_parse_pitch(bufptr, lastChar);
            if (LangTokenType::Error == primitive.type) {
                if (is_char_type(LangCharType::ListEnd, *buf)) {
                    memcpy(list.ptr, &primitive, sizeof(LangPrimitive));
                    goto Finalise_list;
                }
            }
            memcpy(list.ptr, &primitive, sizeof(LangPrimitive));
            list.ptr++;
        } else if(is_char_type(LangCharType::Digit, **bufptr)) {
            LangPrimitive primitive = lang_parse_length(bufptr);
            if (LangTokenType::Error == primitive.type) {
                if (is_char_type(LangCharType::ListEnd, *buf)) {
                    goto Finalise_list;
                }
            }
            memcpy(list.ptr++, &primitive, sizeof(LangPrimitive));
        } else if(is_char_type(LangCharType::ListEnd, **bufptr)) {
            goto Finalise_list;
        } else if(is_whitespace_char(*buf)) {
            ++buf;
            continue;
        }
    }

Finalise_list:
    return;
}
