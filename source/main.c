#include <stdio.h>
#include <stddef.h>
#include <sysexits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "common.h"
#include "config.h"
#include "parser.h"
#include "strings.h"
#include "table.h"

GtkTextBuffer *shell_buffer = NULL;

static char *readFile(FILE *file) {
  fseek(file, 0L, SEEK_END);
  size_t filesize = ftell(file);
  rewind(file);

  char *buffer = malloc(filesize + 1);

  if (buffer == NULL) {
    fprintf(stderr, "Ran out of memory loading file into buffer.\n");
    exit(EX_IOERR);
  }

  size_t bytesRead = fread(buffer, sizeof(char), filesize, file);
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

void runCommand(GtkWidget *widget, gpointer data) {
  char *command = parseCommand(data);

  int mypipe[2];

  if (pipe(mypipe)) {
    fprintf(stderr, "Pipe failed!\n");
    return;
  }
  
  pid_t pid = fork();
  if (pid == -1) {
    fprintf(stderr, "Failed to open shell process.");
  } else if (pid == 0) {
    close(mypipe[0]); /* Close the read end of the pipe. */
    dup2(mypipe[1], 1);
    execl("/bin/sh", "sh", "-c", command, (char *) NULL);
  } else {
    close(mypipe[1]); /* Close the write end of the pipe. */
    FILE *input = fdopen(mypipe[0], "r");

    char *line = NULL;
    size_t n = 0;
    GtkTextIter iter;
    gtk_text_buffer_set_text(shell_buffer, "", -1);
    while ((getline(&line, &n, input)) != -1) {    
    gtk_text_buffer_get_end_iter(shell_buffer, &iter);
    gtk_text_buffer_insert(shell_buffer, &iter, line, -1);
    }
    free(line);
  }
}

void toggleCommand(GtkWidget *widget, gpointer key) {
  bool active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  bool success = enableVariable(key, active);
  if (!success) {
    fprintf(stderr, "Error toggling variable '%s'! Maybe it wasn't declared?", (char *)key);
  }
}

void toggleWidget(GtkWidget *widget, gpointer name) {
  bool active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  bool success = setSensitiveWidget(name, active);
  if (!success) {
    fprintf(stderr, "Error toggling widget '%s'! Maybe it wasn't named?", (char *)name);
  }
}

void updateVariable(GtkEntryBuffer *text, guint position, gchar *chars, guint n_chars,  gpointer key) {
  const char *buffer = gtk_entry_buffer_get_text(text);
  bool success = setVariable(key, buffer);
  /* if (!success) {
    fprintf(stderr, "Error updating variable '%s' from text buffer!", (char *)key);
    } */
}

void updateConsoleVariable(GObject *text, GParamSpec *pspec, gpointer key) {
  static int check_line = -1; /* Static variable to check if the cursor actually moved a line */
  GtkTextMark *insert = gtk_text_buffer_get_insert(shell_buffer);
  GtkTextIter insert_iter;
  gtk_text_buffer_get_iter_at_mark(shell_buffer, &insert_iter, insert);
  int insert_line = gtk_text_iter_get_line(&insert_iter);
  if (insert_line == check_line) {
    return;
  } else {
    check_line = insert_line;
  }

  GtkTextIter line_start;
  gtk_text_buffer_get_iter_at_line(shell_buffer, &line_start, insert_line);
  GtkTextIter line_end = line_start;
  gtk_text_iter_forward_to_line_end(&line_end);

  static char *prev_chars = NULL;
  if (prev_chars != NULL) {
    free(prev_chars);
  }
  char *new_chars = gtk_text_buffer_get_slice(shell_buffer, &line_start, &line_end, false);
  prev_chars = new_chars;
  setVariable(key, new_chars);
}

static void activate(GtkApplication *app, gpointer userdata) {
  GtkWidget *window;
  GtkWidget *list;
  
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
  
  build(userdata, window);

  gtk_widget_show_all(window);
}

static char *openFile(char *filename) {
  FILE *file = fopen(filename, "rb");

  if (file == NULL) {
    fprintf(stderr, "'%s' file could not be opened.\n", filename);
    exit(EX_IOERR);
  }
  return readFile(file);
}

int main(int argc, char **argv) {
  argc--, argv++;

  if (argc != 1) {
    fprintf(stderr, "Bad usage!");
    exit(EX_USAGE);
  }

  char *source = openFile(argv[0]);
  
  GtkApplication *app;
  int status;

  shell_buffer = gtk_text_buffer_new(NULL);
  
  app = gtk_application_new("com.sktb.sidli", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), source);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  freeStrings();
  free(source);
  return status;
}
