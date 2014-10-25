#include "common.h"
#include "data/type.h"

char *type_repr(int type)
{
    switch(type) {
        case OPEN_BR: case CLOSE_BR: return "bracket";
        case DIGIT: return "digit";
        case SYMBOL: return "symbol";
        default:
            test(0, "No match type representation.");
    }
}
