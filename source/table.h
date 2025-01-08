#ifndef SGIDLS_TABLE
#define SGIDLS_TABLE

#include <gtk/gtk.h>
#include <stdbool.h>

extern void printVariables();
extern char *getVariable(const char *key);
extern bool teachVariable(const char *key);
extern bool setVariable(const char *key, char *value);
extern bool enableVariable(const char *key, bool shouldEnable);
extern bool getEnableVariable(const char *key);

extern GtkWidget *getWidget(const char *name);
extern bool teachWidget(const char *name);
extern bool setWidget(const char *name, GtkWidget *widget);
extern bool setSensitiveWidget(const char *name, bool sensitivity);
#endif
