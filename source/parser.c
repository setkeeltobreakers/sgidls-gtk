#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "scanner.h"
#include "parser.h"
#include "strings.h"
#include "table.h"

typedef struct {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
} Parser;

Token nulltoken = (Token) {TOKEN_NULL, NULL, 0, 0};

Parser parser;
GtkWidget *main_window = NULL;

static void entry(GtkWidget *parent);
static void config_entry();

static void initParser() {
  parser.current = nulltoken;
  parser.previous = nulltoken;
  parser.hadError = false;
  parser.panicMode = false;
}

static void errorAt(Token *token, char *message) {
  if (parser.panicMode) return;

  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    /* Nothing */
  } else {
    fprintf(stderr, " at %.*s", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
  parser.panicMode = true;
}

static void error(char *message) {
  errorAt(&parser.current, message);
}

static void advance() {
  parser.previous = parser.current;

  while (true) {
    parser.current = scanToken();
    //printToken(&parser.current);
    if (parser.current.type != TOKEN_ERROR) return;

    error(parser.current.start);
  }
}

static bool check(tokenType type) {
  return parser.current.type == type;
}

static bool match(tokenType type) {
  if (!check(type)) return false;
  advance();
  return true;
}

static void consume(tokenType type, char *message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  error(message);
}

static void nameWidget(GtkWidget *parent) {
  consume(TOKEN_STRING, "Widget name must be a string!");
  char *name = pluckToken(&parser.previous);
  bool success = setWidget(name, parent);
  if (!success) error("Something went wrong with widget naming!");
}

static void setSensitive(GtkWidget *parent) {
  if (match(TOKEN_TRUE)) {
    gtk_widget_set_sensitive(parent, true);
  } else if (match(TOKEN_FALSE)) {
    gtk_widget_set_sensitive(parent, false);
  }
}

static void setExpandFill(GtkWidget *widget, gpointer box) {
  if (!(GTK_IS_SEPARATOR(widget) || GTK_IS_LABEL(widget))) {
    gtk_box_set_child_packing(GTK_BOX(box), widget, true, true, 0, GTK_PACK_START);
  }
}

static void setExpand(GtkWidget *widget, gpointer box) {
  if (!(GTK_IS_SEPARATOR(widget) || GTK_IS_LABEL(widget))) {
    gtk_box_set_child_packing(GTK_BOX(box), widget, true, false, 0, GTK_PACK_START);
  }
}

static void label(GtkWidget *parent) {
  GtkWidget *da_label;
  
  consume(TOKEN_STRING, "Invalid label text.");
  char *labeltext = pluckToken(&parser.previous);
  da_label = gtk_label_new(labeltext);
  gtk_container_add(GTK_CONTAINER(parent), da_label);
}

static void line(GtkWidget *parent) {
  GtkWidget *da_line;

  advance(); /* It really doesn't matter what you put here, a line is a line */
  da_line = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(parent), da_line);
}

static void bar(GtkWidget *parent) {
  GtkWidget *da_bar;
  advance();
  da_bar = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
  gtk_container_add(GTK_CONTAINER(parent), da_bar);
}

static void variable() {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for variable declaration.");
  consume(TOKEN_STRING, "Missing variable name for variable declaration.");

  char *name = pluckToken(&parser.previous);

  consume(TOKEN_COLON, "Missing colon.");
  consume(TOKEN_STRING, "Missing variable value for variable declaration.");

  char *value = pluckToken(&parser.previous);

  bool enable = true;
  if (match(TOKEN_COMMA)) {
    consume(TOKEN_ENABLE, "Missing 'enable' keyword.");
    consume(TOKEN_COLON, "Missing colon.");
    if (!match(TOKEN_TRUE)) {
      consume(TOKEN_FALSE, "'enable' can only be true or false!");
      enable = false;
    }
  }
  
  consume(TOKEN_CLOSE_OBJECT, "Missing closing curly brace for variable declaration.");

  bool success = setVariable(name, value);
  if (!success) error("Something went wrong in variable declaration!");
  success = enableVariable(name, enable);
  if (!success) error("Something went wrong in variable enabling!");
}

static void textbox(GtkWidget *parent) {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for textbox description.");

  GtkWidget *textbox;
  GtkEntryBuffer *text;
  
  bool hasVariable = false;

  text = gtk_entry_buffer_new(NULL, -1);
  textbox = gtk_entry_new_with_buffer(text);

  gtk_container_add(GTK_CONTAINER(parent), textbox);
  
  while (!match(TOKEN_CLOSE_OBJECT)) {
    advance();
    switch (parser.previous.type) {
    case TOKEN_VARIABLE: {
      consume(TOKEN_COLON, "Missing colon.");
      consume(TOKEN_STRING, "Invalid variable name.");

      hasVariable = true;
      char *variable = pluckToken(&parser.previous);
      g_signal_connect(text, "inserted-text", G_CALLBACK(updateVariable), variable);
    } break;
    case TOKEN_NAME: {
      consume(TOKEN_COLON, "Missing colon.");
      nameWidget(textbox);
    } break;
    default: error("Invalid keyword for textbox description.");
    }

    if (!check(TOKEN_CLOSE_OBJECT)) consume(TOKEN_COMMA, "Missing comma.");
  }
}

static void checkbox(GtkWidget *parent) {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for checkbox description.");

  GtkWidget *checkbox;

  bool hasLabel = false;
  bool hasConnection = false;
  
  checkbox = gtk_check_button_new();
  gtk_container_add(GTK_CONTAINER(parent), checkbox);

  while (!match(TOKEN_CLOSE_OBJECT)) {
    advance();
    switch (parser.previous.type) {
    case TOKEN_LABEL: {
      if (hasLabel) error("Check button already has a label!");
      consume(TOKEN_COLON, "Missing colon.");
      /*consume(TOKEN_STRING, "Invalid check button label.");
      
      char *labelChars = pluckToken(&parser.previous);
      GtkWidget *label;
      
      label = gtk_label_new(labelChars);
      gtk_container_add(GTK_CONTAINER(checkbox), label); */
      label(checkbox);
    } break;
    case TOKEN_VARIABLE: {
      hasConnection = true;
      
      consume(TOKEN_COLON, "Missing colon.");
      consume(TOKEN_STRING, "Invalid variable name.");

      char *variable = pluckToken(&parser.previous);
      bool isEnabled = getEnableVariable(variable);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), isEnabled);
      g_signal_connect(checkbox, "toggled", G_CALLBACK(toggleCommand), variable);
    } break;
    case TOKEN_ENABLE: {
      hasConnection = true;
      
      consume(TOKEN_COLON, "Missing colon.");
      consume(TOKEN_STRING, "Invalid widget name.");

      char *widget = pluckToken(&parser.previous);
      g_signal_connect(checkbox, "toggled", G_CALLBACK(toggleWidget), widget);
    } break;
    default: error("Invalid checklist keyword.");
    }
    
    if (!check(TOKEN_CLOSE_OBJECT)) consume(TOKEN_COMMA, "Missing comma.");
  }

  if (!hasConnection) error("Check button with no connections!");
}

static void checklist(GtkWidget *parent) {
  consume(TOKEN_OPEN_ARRAY, "Missing opening square bracket for checklist description.");

  GtkWidget *list;
  
  list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_add(GTK_CONTAINER(parent), list);
  
  while (!match(TOKEN_CLOSE_ARRAY)) {
    checkbox(list);
    if (!check(TOKEN_CLOSE_ARRAY)) consume(TOKEN_COMMA, "Missing comma.");
  }

  gtk_container_foreach(GTK_CONTAINER(list), setExpand, list);
}

static void console(GtkWidget *parent) {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for console description!");

  GtkWidget *textview;
  GtkWidget *scrollwindow;

  scrollwindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollwindow), 200);
  gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrollwindow), 200);
  textview = gtk_text_view_new_with_buffer(shell_buffer);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), false);
  gtk_container_add(GTK_CONTAINER(parent), scrollwindow);
  gtk_container_add(GTK_CONTAINER(scrollwindow), textview);

  while (!match(TOKEN_CLOSE_OBJECT)) {
    advance();
    switch (parser.previous.type) {
    case TOKEN_VARIABLE: {
      consume(TOKEN_COLON, "Missing colon.");
      consume(TOKEN_STRING, "Invalid variable name");

      char *variable = pluckToken(&parser.previous);
      g_signal_connect(shell_buffer, "notify::cursor-position", G_CALLBACK(updateConsoleVariable), variable);
    } break;
    default: error("Invalid keyword for console description.");
    }
  }
}

static void button(GtkWidget *parent) {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for button description.");

  bool hasLabel = false;
  bool hasCommand = false;

  GtkWidget *button;
  GtkWidget *button_box;

  button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(parent), button_box);

  button = gtk_button_new();
  
  while (!match(TOKEN_CLOSE_OBJECT)) {
    advance();
    switch (parser.previous.type) {
    case TOKEN_LABEL: {
      if (hasLabel) error("Buttons can only have one label!");
      consume(TOKEN_COLON, "Missing colon.");
      /*consume(TOKEN_STRING, "Value not valid label.");
      char *labelString = pluckToken(&parser.previous);
      gtk_button_set_label(GTK_BUTTON(button), labelString); */
      label(button);
      hasLabel = true;
    } break;
    case TOKEN_COMMAND: {
      if (hasCommand) error("Buttons can only have one command!");
      consume(TOKEN_COLON, "Missing colon.");
      if (match(TOKEN_EXIT)) {
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy), main_window);
      } else {
	consume(TOKEN_STRING, "Value not valid command.");
	char *command = pluckToken(&parser.previous);
	//char *commandString = parseCommand(command);
	g_signal_connect(button, "clicked", G_CALLBACK(runCommand), command);
      }
      hasCommand = true;
    } break;
    case TOKEN_NAME: {
      consume(TOKEN_COLON, "Missing colon.");
      nameWidget(button);
    } break;
    case TOKEN_ENABLE: {
      consume(TOKEN_COLON, "Missing colon.");
      setSensitive(button);
    } break;
    default: error("Invalid key for button object.");
    }

    if (!check(TOKEN_CLOSE_OBJECT)) consume(TOKEN_COMMA, "Missing comma.");
  }

  if (!hasLabel) error("No label set for button!");
  if (!hasCommand) error("No command set for button!");

  gtk_container_add(GTK_CONTAINER(button_box), button);
}

static void row(GtkWidget *parent) {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for row description.");

  bool hasEntries = false;

  GtkWidget *row;
  row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_container_set_border_width(GTK_CONTAINER(row), 5);
  gtk_container_add(GTK_CONTAINER(parent), row);
  
  while (!match(TOKEN_CLOSE_OBJECT)) {
    entry(row);
  }

  gtk_container_foreach(GTK_CONTAINER(row), setExpandFill, row);
}

static void list(GtkWidget *parent) {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for list description.");

  bool hasEntries = false;

  GtkWidget *list;

  list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_set_border_width(GTK_CONTAINER(list), 5);
  gtk_container_add(GTK_CONTAINER(parent), list);

  while (!match(TOKEN_CLOSE_OBJECT)) {
    entry(list);
  }

  //gtk_container_foreach(GTK_CONTAINER(list), setExpandFill, list);
}

static void column(GtkWidget *parent) {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for list description.");

  bool hasEntries = false;

  GtkWidget *list;

  list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_set_border_width(GTK_CONTAINER(list), 5);
  gtk_container_add(GTK_CONTAINER(parent), list);

  while (!match(TOKEN_CLOSE_OBJECT)) {
    entry(list);
  }

  gtk_container_foreach(GTK_CONTAINER(list), setExpandFill, list);
}

static void window(GtkWidget *parent) {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for window description.");
  entry(parent);
  consume(TOKEN_CLOSE_OBJECT, "Missing closing curly brace for window description.");
}

static void entry(GtkWidget *parent) {
  advance();
  switch (parser.previous.type) {
  case TOKEN_BUTTON: {
    consume(TOKEN_COLON, "Missing colon.");
    button(parent);
  } break;
  case TOKEN_LIST: {
    consume(TOKEN_COLON, "Missing colon.");
    list(parent);
  } break;
  case TOKEN_CHECKLIST: {
    consume(TOKEN_COLON, "Missing colon.");
    checklist(parent);
  } break;
  case TOKEN_TEXTBOX: {
    consume(TOKEN_COLON, "Missing colon.");
    textbox(parent);
  } break;
  case TOKEN_LABEL: {
    consume(TOKEN_COLON, "Missing colon.");
    label(parent);
  } break;
  case TOKEN_HLINE: {
    consume(TOKEN_COLON, "Missing colon.");
    line(parent);
  } break;
  case TOKEN_CONSOLE: {
    consume(TOKEN_COLON, "Missing colon.");
    console(parent);
  } break;
  case TOKEN_ROW: {
    consume(TOKEN_COLON, "Missing colon");
    row(parent);
  } break;
  case TOKEN_VLINE: {
    consume(TOKEN_COLON, "Missing colon");
    bar(parent);
  } break;
  case TOKEN_COLUMN: {
    consume(TOKEN_COLON, "Missing colon");
    column(parent);
  } break;
  default: error("Invalid entry key.");
  }
  if (!check(TOKEN_CLOSE_OBJECT)) consume(TOKEN_COMMA, "Missing comma.");
}

static void config_entry() {
  advance();
  switch (parser.previous.type) {
  case TOKEN_NAME: {
    consume(TOKEN_COLON, "Missing colon.");
    consume(TOKEN_STRING, "Invalid window name.");
    gtk_window_set_title(GTK_WINDOW(main_window), pluckToken(&parser.previous));
  } break;
  case TOKEN_VARIABLE: {
    consume(TOKEN_COLON, "Missing colon.");
    variable();
  } break;
  default: error("Invalid config key.");
  }
  if (!check(TOKEN_CLOSE_OBJECT)) consume(TOKEN_COMMA, "Missing comma.");
}

static void config() {
  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for window configuration.");
  while (!match(TOKEN_CLOSE_OBJECT)) {
    config_entry();
  }
}

void build(char *source, GtkWidget *app_window) {
  main_window = app_window;
  
  initScanner(source);
  initParser();
  advance();

  consume(TOKEN_OPEN_OBJECT, "Missing opening curly brace for description file.");
  if (match(TOKEN_CONFIG)) {
    consume(TOKEN_COLON, "Missing colon.");
    config();
    consume(TOKEN_COMMA, "Missing comma.");
  }
  consume(TOKEN_WINDOW, "Missing window keyword.");
  consume(TOKEN_COLON, "Missing colon.");
  window(app_window);
  consume(TOKEN_CLOSE_OBJECT, "Missing closing curly brace for description file.");
  consume(TOKEN_EOF, "File continues after window object ends!");
  if (parser.hadError) {
    fprintf(stderr, "Parser error!\n");
    exit(1);
  }
}
