enum struct LangCharType {  // The actual char that can appear in buffer
    Error             = 0x0000,
    SemitoneMod       = 0x0001,
    NoteName          = 0x0002,
    Letter            = 0x0004,
    Digit             = 0x0010,
    Minus             = 0x0020,
    Symbol            = 0x0040,
    Quote             = 0x0080,
    ListBegin         = 0x0100,
    ListEnd           = 0x0200,
    BlockBegin        = 0x0400,
    BlockEnd          = 0x0800,
    LengthPitchConcat = 0x1000,
};

// Length can have at most 2 digits followed by a length modifier.
enum struct LangCharLenType {
    Error          = 0x000,
    Digit1         = 0x001,
    Digit2         = 0x002,
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

enum struct LangErrorType {
    None                = 0x000,
    InvalidLengthChar   = 0x001,
    InvalidPitchChar    = 0x002,
    InvalidVelocityChar = 0x004,
    OutOfBoundsPitch    = 0x008,
    OutOfBoundsLength   = 0x010,
    OutOfBoundsVelocity = 0x020,
};

enum LengthMod {
    Normal = 0, Dotted, Triplet
};

// LangPrimitive should contain all values that will eventually be converted to midi.
// These include length, pitch, and velocity.
//
// Within the primitive, length is the combination of length and modifier. The final
// midi length value will be calculated during convesion to midi.
//
// Unrepresented values are represented with a -1
struct LangPrimitive {
    LangErrorType error = LangErrorType::None;
    int pitch           = -1;
    int length          = -1;
    LengthMod lengthMod = LengthMod::Normal;
    int velocity        = -1;
};

bool is_only_pitch(LangPrimitive &prim)
{
    return prim.pitch > -1 && prim.length == -1 && prim.velocity == -1;
}

bool is_only_length(LangPrimitive &prim)
{
    return prim.length > -1 && prim.pitch == -1 && prim.velocity == -1;
}

bool is_only_velocity(LangPrimitive &prim)
{
    return prim.velocity > -1 && prim.pitch == -1 && prim.length == -1;
}

int lang_char_len_pos(char &x) {
    switch (x) {
        case '1':
        case '3':
        case '8': return (int)LangCharLenType::Digit1;
        case '2':
        case '4':
        case '6': return (int)LangCharLenType::Digit1 | (int)LangCharLenType::Digit2;
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
        case '.': return (int)LangCharType::Symbol | (int)LangCharType::LengthPitchConcat;
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

#define lang_abstract_parse_char(prim, SWITCH_CASES) \
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
                return prim; \
            } else { \
                rulePos = rulePos->alt; \
                continue; \
            } \
        }

LangPrimitive lang_parse_length(char **buf)
{
    LangPrimitive prim;
    if (!is_char_type(LangCharType::Digit, **buf)) {
lang_parse_length_default:
        prim.error = LangErrorType::InvalidLengthChar;
        return prim;
    }

    LangLenToken parseRules[3] = {
        { LangCharLenType::Digit1,      &parseRules[1], NULL           }, // 0
        { LangCharLenType::Digit2,      NULL,           &parseRules[2] }, // 1
        { LangCharLenType::LengthMod,   NULL,           NULL           }, // 2
    };

    LangLenToken *rulePos = parseRules;
    char lenStr[2];
    int lenVal = -1;
    LangTokenType type = LangTokenType::LengthNormal;

    while(!is_whitespace_char(**buf)) {
        char ch = **buf;
        char *lenValPos = lenStr;
        lang_abstract_parse_char(prim, {
            case LangCharLenType::Digit1:
            case LangCharLenType::Digit2: {
                *lenValPos++ = ch;
            } break;
            case LangCharLenType::LengthMod: {
                if ('d' == ch) {
                    prim.lengthMod = LengthMod::Dotted;
                } else if ('t' == ch) {
                    prim.lengthMod = LengthMod::Triplet;
                }
            } break;
            default: { goto lang_parse_length_default; }
        });
    }
    prim.length = atoi(lenStr);
    return prim;
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
    LangPrimitive prim;
    if (!is_char_type(LangCharType::Letter, **buf)) {
        prim.error = LangErrorType::InvalidPitchChar;
        return prim;
    }

    LangToken parseRules[4] = {
        { LangCharType::NoteName,    &parseRules[1], NULL     }, // 0
        { LangCharType::Minus,       &parseRules[2], &parseRules[2] }, // 1
        { LangCharType::Digit,       &parseRules[3], NULL     }, // 2
        { LangCharType::SemitoneMod, NULL,           NULL     }, // 3
    };

    LangToken *rulePos = parseRules;
    int octave = 0;
    int octaveSign = 1;


    bool lastCharCheck = nullptr != lastChar? *buf != lastChar : true;
    while(!is_whitespace_char(**buf) && lang_last_char_check(*buf, lastChar)) {
        char ch = **buf;
        lang_abstract_parse_char(prim, {
            case LangCharType::NoteName: {
                prim.pitch = pitch_to_midi(ch);
            } break;
            case LangCharType::Minus: {
                octaveSign = -1;
            } break;
            case LangCharType::Digit: {
                octave = atoi(&ch) * 12;
            } break;
            case LangCharType::SemitoneMod: {
                prim.pitch += ch == '#'? 1 : -1;
            } break;
            default: break;
        });
    }
    octave *= octaveSign;
    prim.pitch += octave;
    if (-1 < prim.pitch && prim.pitch < 128) {
        return prim;
    }
    prim.error = LangErrorType::OutOfBoundsPitch;
    return prim;
}

inline LangPrimitive lang_parse_pitch(char *buf)
{
    char **bufptr = &buf;
    return lang_parse_pitch(bufptr);
}

LangPrimitive lang_parse_note(char **buf, char *lastChar = nullptr)
{
    int posPitch = -1;
    // Note is a length followed by a dot followed by a pitch.
    // TODO:
    // * Parse buffer for correctness. Record location of parts as you go
    // * Put everything before dot into temp buffer to be sent do length parser
    // TODO
    // * If char begin with a length, send it off to be processed for length.
    //   * Then send it off to be processed for pitch.
    // * If char begins with a pitch, it's not a note or can we work out the
    //   length?
    //
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

inline bool is_ok(LangErrorType error) {
    return error == LangErrorType::None;
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
            if (is_ok(primitive.error)) {
                if (is_char_type(LangCharType::ListEnd, *buf)) {
                    memcpy(list.ptr, &primitive, sizeof(LangPrimitive));
                    goto Finalise_list;
                }
            }
            memcpy(list.ptr, &primitive, sizeof(LangPrimitive));
            list.ptr++;
        } else if(is_char_type(LangCharType::Digit, **bufptr)) {
            LangPrimitive primitive = lang_parse_length(bufptr);
            if (is_ok(primitive.error)) {
                if (is_char_type(LangCharType::ListEnd, *buf)) {
                    goto Finalise_list;
                }
                if (is_char_type(LangCharType::LengthPitchConcat, *buf)) {
                    // TODO: next part should be a pitch
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
