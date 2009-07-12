
#ifndef __GMPC_TEST_PLUGIN_H__
#define __GMPC_TEST_PLUGIN_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <gmpc_easy_download.h>
#include <stdlib.h>
#include <string.h>
#include <metadata.h>
#include <libmpd/libmpdclient.h>
#include <libmpd/libmpd.h>
#include <gmpc-plugin.h>

G_BEGIN_DECLS


#define GMPC_META_DATA_TYPE_EDIT_WINDOW (gmpc_meta_data_edit_window_get_type ())
#define GMPC_META_DATA_EDIT_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GMPC_META_DATA_TYPE_EDIT_WINDOW, GmpcMetaDataEditWindow))
#define GMPC_META_DATA_EDIT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GMPC_META_DATA_TYPE_EDIT_WINDOW, GmpcMetaDataEditWindowClass))
#define GMPC_META_DATA_IS_EDIT_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GMPC_META_DATA_TYPE_EDIT_WINDOW))
#define GMPC_META_DATA_IS_EDIT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GMPC_META_DATA_TYPE_EDIT_WINDOW))
#define GMPC_META_DATA_EDIT_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GMPC_META_DATA_TYPE_EDIT_WINDOW, GmpcMetaDataEditWindowClass))

typedef struct _GmpcMetaDataEditWindow GmpcMetaDataEditWindow;
typedef struct _GmpcMetaDataEditWindowClass GmpcMetaDataEditWindowClass;
typedef struct _GmpcMetaDataEditWindowPrivate GmpcMetaDataEditWindowPrivate;

#define GMPC_TYPE_TEST_PLUGIN (gmpc_test_plugin_get_type ())
#define GMPC_TEST_PLUGIN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GMPC_TYPE_TEST_PLUGIN, GmpcTestPlugin))
#define GMPC_TEST_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GMPC_TYPE_TEST_PLUGIN, GmpcTestPluginClass))
#define GMPC_IS_TEST_PLUGIN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GMPC_TYPE_TEST_PLUGIN))
#define GMPC_IS_TEST_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GMPC_TYPE_TEST_PLUGIN))
#define GMPC_TEST_PLUGIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GMPC_TYPE_TEST_PLUGIN, GmpcTestPluginClass))

typedef struct _GmpcTestPlugin GmpcTestPlugin;
typedef struct _GmpcTestPluginClass GmpcTestPluginClass;
typedef struct _GmpcTestPluginPrivate GmpcTestPluginPrivate;

struct _GmpcMetaDataEditWindow {
	GtkWindow parent_instance;
	GmpcMetaDataEditWindowPrivate * priv;
};

struct _GmpcMetaDataEditWindowClass {
	GtkWindowClass parent_class;
};

struct _GmpcTestPlugin {
	GmpcPluginBase parent_instance;
	GmpcTestPluginPrivate * priv;
};

struct _GmpcTestPluginClass {
	GmpcPluginBaseClass parent_class;
};


GType gmpc_meta_data_edit_window_get_type (void);
void gmpc_meta_data_edit_window_image_downloaded (GmpcMetaDataEditWindow* self, const GEADAsyncHandler* handle, GEADStatus status);
void gmpc_meta_data_edit_window_callback (GmpcMetaDataEditWindow* self, void* handle, const char* plugin_name, GList* list);
void gmpc_meta_data_edit_window_store_image (GmpcMetaDataEditWindow* self, const GEADAsyncHandler* handle, GEADStatus status);
void gmpc_meta_data_edit_window_destroy_popup (GmpcMetaDataEditWindow* self, GtkButton* button);
void gmpc_meta_data_edit_window_refresh_query (GmpcMetaDataEditWindow* self, GtkButton* button);
GmpcMetaDataEditWindow* gmpc_meta_data_edit_window_new (const mpd_Song* song, MetaDataType type);
GmpcMetaDataEditWindow* gmpc_meta_data_edit_window_construct (GType object_type, const mpd_Song* song, MetaDataType type);
void gmpc_meta_data_edit_window_b_cancel (GmpcMetaDataEditWindow* self);
GType gmpc_test_plugin_get_type (void);
void gmpc_test_plugin_menu_activated_album (GmpcTestPlugin* self, GtkMenuItem* item);
GmpcTestPlugin* gmpc_test_plugin_new (void);
GmpcTestPlugin* gmpc_test_plugin_construct (GType object_type);

static const gint GMPC_TEST_PLUGIN_version[] = {0, 0, 2};

G_END_DECLS

#endif