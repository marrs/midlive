struct ExTestInt { int left; int right; };
struct ExTestStr { char *left; char *right; };
struct ExTestBool { bool left; bool right; };

union test {
    int iVal;
    bool bVal;
    char *sVal;

    test(int x)
    {
        iVal = x;
    }

    test(char *x)
    {
        sVal = x;
    }

    test(bool *x)
    {
        bVal = x;
    }

    void eq(int x)
    {
        bool result = iVal == x;
        if (!result) {
            printf("\n  %d does not equal %d\n", iVal, x);
            throw ExTestInt {iVal, x};
        }
        wprintf(L"%lc", (wchar_t)0x2713);
    }

    void eq(char *x)
    {
        bool result = sVal == x;
        if (!result) {
            printf("\n  %s does not equal %s\n", sVal, x);
            throw ExTestStr {sVal, x};
        }
    }

    void eq(bool x)
    {
        bool result = bVal == x;
        if (!result) {
            printf("\n  %d does not equal %d\n", bVal, x);
            throw ExTestBool {bVal, x};
        }
    }
};

#define test_case(DESC, BLOCK) printf("\n%s\t",DESC); try BLOCK \
    catch (ExTestInt x) { assert(x.left == x.right); } \
    catch (ExTestStr x) { assert(x.left == x.right); } \
    catch (ExTestBool x) { assert(x.left == x.right); }
