#ifndef __TYPE_H__
#define __TYPE_H__

enum {
    NOTYPE, DIGIT, 
    IDENTIFIER,
    OPEN_BR, CLOSE_BR,
};

char *type_repr(int type);

#endif
