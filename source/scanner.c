#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "scanner.h"
#include "config.h"

typedef struct {
  char *start;
  char *current;
  int line;
} Scanner;

static Scanner scanner;

void printToken(Token *token) {
  printf("%d %.*s\n", token->type, token->length, token->start);
}

void initScanner(char *source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

static Token makeToken(tokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int) (scanner.current - scanner.start);
  token.line = scanner.line;

  return token;
}

static Token errorToken(char *message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int) strlen(message);
  token.line = scanner.line;

  return token;
}

static bool isAtEnd() {
  return *scanner.current == '\0';
}

static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

static int match(char expected) {
  if (isAtEnd()) return false;
  if (*scanner.current != expected) return false;
  scanner.current++;
  return true;
}

static char peek() {
  return *scanner.current;
}

static void skipWhitespace() {
  while (true) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      advance();
      scanner.line++;
      break;
    case '#': /* Comments */
      while (peek() != '\n' && !isAtEnd()) advance(); /* Consume all characters except for \n in a comment */
      break;
    default:
      return;
    }
  }
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isDigit(char c) {
  return (c >= '0' && c <= '9');
}

static Token number() {
  while (isDigit(peek())) advance();

  if (peek() == '.') advance();

  while (isDigit(peek())) advance();

  return makeToken(TOKEN_NUMBER);
}

static Token string() {
  while (peek() != '"') {
    if (isAtEnd()) return errorToken("Unterminated string. Somebody call Arnold!");

    if (peek() == '\n') scanner.line++;

    advance();
  }

  advance();
  return makeToken(TOKEN_STRING);
}

static tokenType checkKeyword(int start, int length, char *rest, tokenType type) {
  if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length) == 0) return type;

  return TOKEN_ERROR;
}

static tokenType keywordType() {
  if (scanner.current - scanner.start == 1) return TOKEN_ERROR; /* No single character keywords. And I mean it!*/

  switch (*scanner.start) {
  case 'b': return checkKeyword(1, 5, "utton", TOKEN_BUTTON);
  case 'c': switch (scanner.start[1]) {
    case 'h': return checkKeyword(2, 7, "ecklist", TOKEN_CHECKLIST);
    case 'o': switch (scanner.start[2]) {
      case 'l': return checkKeyword(3, 3, "umn", TOKEN_COLUMN);
      case 'm': return checkKeyword(3, 4, "mand", TOKEN_COMMAND);
      case 'n': switch (scanner.start[3]) {
	case 'f': return checkKeyword(4, 2, "ig", TOKEN_CONFIG);
	case 's': return checkKeyword(4, 3, "ole", TOKEN_CONSOLE);
	} break;
      } break;
    } break;
  case 'e': switch (scanner.start[1]) {
    case 'n': return checkKeyword(2, 4, "able", TOKEN_ENABLE);
    case 'x': return checkKeyword(2, 2, "it", TOKEN_EXIT);
    } break;
  case 'f': return checkKeyword(1, 4, "alse", TOKEN_FALSE);
  case 'h': return checkKeyword(1, 4, "line", TOKEN_HLINE);
  case 'l': switch (scanner.start[1]) {
    case 'a': return checkKeyword(2, 3, "bel", TOKEN_LABEL);
    case 'i': return checkKeyword(2, 2, "st", TOKEN_LIST);
    } break;
  case 'n': switch (scanner.start[1]) {
    case 'u': return checkKeyword(2, 2, "ll", TOKEN_NULL);
    case 'a': return checkKeyword(2, 2, "me", TOKEN_NAME);
    } break;
  case 'r': return checkKeyword(1, 2, "ow", TOKEN_ROW);
  case 't': switch (scanner.start[1]) {
    case 'e': return checkKeyword(2, 5, "xtbox", TOKEN_TEXTBOX);
    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
  } break;
  case 'v': switch (scanner.start[1]) {
    case 'a': return checkKeyword(2, 6, "riable", TOKEN_VARIABLE);
    case 'l': return checkKeyword(2, 3, "ine", TOKEN_VLINE);
    }
  case 'w': return checkKeyword(1, 5, "indow", TOKEN_WINDOW);
  }

  return TOKEN_ERROR;
}

static Token keyword() {
  while (isAlpha(peek()) || isDigit(peek())) advance();

  tokenType type = keywordType();

  if (type == TOKEN_ERROR) return errorToken("Unknown keyword");
  
  return makeToken(type);
}

Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;

  if (isAtEnd()) return makeToken(TOKEN_EOF);

  char c = advance();

  if (isAlpha(c)) return keyword();
  if (isDigit(c)) return number();

  switch (c) {
  case '{': return makeToken(TOKEN_OPEN_OBJECT);
  case '}': return makeToken(TOKEN_CLOSE_OBJECT);
  case '[': return makeToken(TOKEN_OPEN_ARRAY);
  case ']': return makeToken(TOKEN_CLOSE_ARRAY);
  case ':': return makeToken(TOKEN_COLON);
  case ',': return makeToken(TOKEN_COMMA);
  case '"': return string();
  }

  return errorToken("Unexpected character.");
}
