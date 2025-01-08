#ifndef SGIDLS_COMMON
#define SGIDLS_COMMON

#include <stdlib.h>
#include <stddef.h>
#include <gtk/gtk.h>

extern void runCommand(GtkWidget *widget, gpointer data);
extern void toggleCommand(GtkWidget *widget, gpointer key);
extern void toggleWidget(GtkWidget *widget, gpointer name);
extern void updateVariable(GtkEntryBuffer *text, guint position, gchar *chars, guint n_chars, gpointer key);

extern GtkTextBuffer *shell_buffer;

static inline void *allocate(size_t size, char *err_message) {
  void *result = malloc(size);

  if (result == NULL) {
    fprintf(stderr, "%s\n", err_message);
    exit(1);
  }

  return result;
}

#endif
