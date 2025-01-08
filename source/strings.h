#ifndef SGIDLS_STRINGS
#define SGIDLS_STRINGS

#include "scanner.h"

extern char *pluckToken(Token *token);
extern void freeStrings();
extern char *parseCommand(char *command);

#endif
