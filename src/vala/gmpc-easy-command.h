
#ifndef __GMPC_EASY_COMMAND_H__
#define __GMPC_EASY_COMMAND_H__

#include <glib.h>
#include <gmpc-plugin.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define GMPC_EASY_TYPE_COMMAND (gmpc_easy_command_get_type ())
#define GMPC_EASY_COMMAND(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GMPC_EASY_TYPE_COMMAND, GmpcEasyCommand))
#define GMPC_EASY_COMMAND_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GMPC_EASY_TYPE_COMMAND, GmpcEasyCommandClass))
#define GMPC_EASY_IS_COMMAND(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GMPC_EASY_TYPE_COMMAND))
#define GMPC_EASY_IS_COMMAND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GMPC_EASY_TYPE_COMMAND))
#define GMPC_EASY_COMMAND_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GMPC_EASY_TYPE_COMMAND, GmpcEasyCommandClass))

typedef struct _GmpcEasyCommand GmpcEasyCommand;
typedef struct _GmpcEasyCommandClass GmpcEasyCommandClass;
typedef struct _GmpcEasyCommandPrivate GmpcEasyCommandPrivate;

struct _GmpcEasyCommand {
	GmpcPluginBase parent_instance;
	GmpcEasyCommandPrivate * priv;
	GtkListStore* store;
};

struct _GmpcEasyCommandClass {
	GmpcPluginBaseClass parent_class;
};

/**
     * This function is called when the user entered a line matching this entry.
     * param data the user data passed.
     * param param a string with the extra parameters passed to the command
     */
typedef void (*GmpcEasyCommandCallback) (void* data, const char* param, void* user_data);

GType gmpc_easy_command_get_type (void);
guint gmpc_easy_command_add_entry (GmpcEasyCommand* self, const char* name, const char* pattern, const char* hint, GmpcEasyCommandCallback* callback, void* userdata);
void gmpc_easy_command_popup (GmpcEasyCommand* self);
void gmpc_easy_command_help_window_destroy (GtkDialog* window, gint response);
void gmpc_easy_command_help_window (void* data, const char* param);
GmpcEasyCommand* gmpc_easy_command_new (void);
GmpcEasyCommand* gmpc_easy_command_construct (GType object_type);


G_END_DECLS

#endif
