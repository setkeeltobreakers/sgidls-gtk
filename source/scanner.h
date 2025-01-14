#ifndef SGIDLS_SCANNER
#define SGIDLS_SCANNER

typedef enum {
  TOKEN_NULL, /* The lonely null */

  /* Opening and closing JSON structs */
  TOKEN_OPEN_OBJECT, TOKEN_CLOSE_OBJECT, TOKEN_OPEN_ARRAY, TOKEN_CLOSE_ARRAY,

  /* JSON Delimeters */
  TOKEN_COLON, TOKEN_COMMA,

  /* SGIDL keywords */
  TOKEN_BUTTON, TOKEN_LABEL, TOKEN_COMMAND, TOKEN_EXIT, TOKEN_LIST, TOKEN_NAME,
  TOKEN_VARIABLE, TOKEN_WINDOW, TOKEN_CONFIG, TOKEN_CHECKLIST, TOKEN_ENABLE,
  TOKEN_TEXTBOX, TOKEN_HLINE, TOKEN_CONSOLE, TOKEN_ROW, TOKEN_VLINE, TOKEN_COLUMN,
  
  /* Literals */
  TOKEN_STRING, TOKEN_NUMBER, TOKEN_TRUE, TOKEN_FALSE,

  /* Special stuff */
  TOKEN_ERROR, TOKEN_EOF
} tokenType;

typedef struct {
  tokenType type;
  char *start;
  int length;
  int line;
} Token;

void printToken(Token *token);
void initScanner(char *source);
Token scanToken();

#endif
