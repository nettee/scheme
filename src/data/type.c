#include "common.h"
#include "data/type.h"

char *type_repr(int type)
{
    switch(type) {
        case OPEN_BR: case CLOSE_BR: return "bracket";
        case DIGIT: return "digit";
        case IDENTIFIER: return "identifier";
        default:
            test(0, "No match type representation.");
    }
}
