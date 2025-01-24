#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "scanner.h"
#include "table.h"

typedef struct LinkedString LinkedString;

struct LinkedString {
  char *chars;
  LinkedString *next;
}; /* Create a linked list of char * to ensure that all strings are remembered and properly disposed of on program exit. */

LinkedString *root = NULL;

static LinkedString *pluckString(char *source, int length) {
  LinkedString *string = malloc(sizeof(LinkedString));
  char *heapChars = malloc((length + 1) * sizeof(char));
  memcpy(heapChars, source, length);
  heapChars[length] = '\0';
  string->chars = heapChars;
  string->next = NULL;
  return string;
}

static void saveString(LinkedString *string) {
  string->next = root;
  root = string;
}

char *pluckToken(Token *token) {
  LinkedString *string = pluckString(token->start + 1, token->length - 2); /* Pluck the string, minus the quotation marks. */
  saveString(string);
  return string->chars;
}

static void freeString(LinkedString *string) {
  free(string->chars);
  free(string);
}
static void freeStringList(LinkedString *start) {
  LinkedString *string = start;
  while (string != NULL) {
    LinkedString *next = string->next;
    freeString(string);
    string = next;
  }
}

static LinkedString *collapseList(LinkedString *da_root) {
  /* This function starts at the head of a linked list of strings and traverses it, 
     combining each one into a single large string.
  */
  int totalLength = 0;
  LinkedString *string = da_root;
  do {
    totalLength += strlen(string->chars);
    string = string->next;
  } while (string != NULL); /* Step 1: traverse the list to get the length of the final string. */

  LinkedString *collapsedString = malloc(sizeof(LinkedString));
  collapsedString->chars = malloc(sizeof(char) * totalLength + 1);
  collapsedString->next = NULL;
  char *c = collapsedString->chars;
  string = da_root;
  do {
    int length = strlen(string->chars);
    strcpy(c, string->chars);
    c += length;
    string = string->next;
  } while (string != NULL); /* Step 2: re-traverse the list, this time copying each string into the final string. */

  collapsedString->chars[totalLength] = '\0';
  return collapsedString; /* Step 3: add string terminator and return string. */
}

static LinkedString *checkVariable(char *variable, int length) {
  LinkedString *varName = pluckString(variable, length);
  LinkedString *varValueString;
  
  char *varValue = getVariable(varName->chars);
  if (varValue == NULL) {
    return NULL;
  } else {
    varValueString = pluckString(varValue, strlen(varValue));
  }
  
  freeString(varName);
  return varValueString;
}

char *parseCommand(char *command) {
  /* This function parses a command string for any known variables, then returns a
     string with found variables expanced. If the function finds a variable name that
     hasn't been previously defined, it will signal an error and terminate the program.
     
     The function does this by first scanning through the command string to find a 
     variable marker, then upon finding one, splits off the initial portion of the 
     string into a LinkedString, finds the expanded variable string and places that 
     in another LinkedString, then places the rest of the command string in another
     LinkedString, then continues searching through the command string until finding
     another variable or reaching the end of the string.
     In doing this, the function builds a linked list of strings which is then collapsed
     into a single string using the collapseList function.
   */

  bool foundVars = false;

  int i = 0;
  char *start = command;
  char c = start[0];

  LinkedString *da_root = NULL;

  LinkedString *string = da_root;
  while (c != '\0') {
    if (c == '%') { /* Couple steps to this process:
		       Step 1: Pluck out the portion of the string prior to the %
		       Step 2: Reset start and i
		       Step 3: Increment until the second %
		       Step 4: Parse variable and expand value 
		       Step 5: Reset start and i one more time
		       Step 6: Keep going.
		    */
      //i++; /* Consume the % */
      if (!foundVars) {
	foundVars = true;
	da_root = pluckString(start, i);
	string = da_root;
      } else {
	LinkedString *newString = pluckString(start + 1, i - 1);
	string->next = newString;
	string = newString;
	//string->chars[0] = ' ';
      } /* Pluck initial portion of string. */
      start = start + i + 1;
      i = 0; /* Reset start and i */
      c = start[i];
      while (c != '%') {
	if (c == '\0') {
	  fprintf(stderr, "Unterminated variable name! Command: %s\n", command);
	  exit(1);
	}
	i++;
	c = start[i];
      }
      LinkedString *newVariable = checkVariable(start, i);
      if (newVariable != NULL) {
	string->next = newVariable;
	string = newVariable;
      }
      start = start + i;
      i = 0;
      //start[0] = ' '; /* Postpend the variable with a space. */
    }

    i++;
    c = start[i];
  }
  
  if (foundVars) {
    LinkedString *endString = pluckString(start, i);
    endString->chars[0] = ' ';
    string->next = endString;
    LinkedString *finalString = collapseList(da_root);
    freeStringList(da_root);
    saveString(finalString);
    return finalString->chars;
  } else {
    return command;
  }
}

void freeStrings() {
  freeStringList(root);
}
