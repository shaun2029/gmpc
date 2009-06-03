
#include <glib.h>
#include <glib-object.h>
#include <config.h>
#include <gtk/gtk.h>
#include <gtktransition.h>
#include <libmpd/libmpd.h>
#include <glib/gi18n-lib.h>
#include <gmpc-connection.h>
#include <libmpd/libmpdclient.h>
#include <stdlib.h>
#include <string.h>
#include <main.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <gdk/gdk.h>
#include <misc.h>
#include <stdio.h>


#define GMPC_FAVORITES_TYPE_LIST (gmpc_favorites_list_get_type ())
#define GMPC_FAVORITES_LIST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GMPC_FAVORITES_TYPE_LIST, GmpcFavoritesList))
#define GMPC_FAVORITES_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GMPC_FAVORITES_TYPE_LIST, GmpcFavoritesListClass))
#define GMPC_FAVORITES_IS_LIST(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GMPC_FAVORITES_TYPE_LIST))
#define GMPC_FAVORITES_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GMPC_FAVORITES_TYPE_LIST))
#define GMPC_FAVORITES_LIST_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GMPC_FAVORITES_TYPE_LIST, GmpcFavoritesListClass))

typedef struct _GmpcFavoritesList GmpcFavoritesList;
typedef struct _GmpcFavoritesListClass GmpcFavoritesListClass;
typedef struct _GmpcFavoritesListPrivate GmpcFavoritesListPrivate;

#define GMPC_FAVORITES_TYPE_BUTTON (gmpc_favorites_button_get_type ())
#define GMPC_FAVORITES_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GMPC_FAVORITES_TYPE_BUTTON, GmpcFavoritesButton))
#define GMPC_FAVORITES_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GMPC_FAVORITES_TYPE_BUTTON, GmpcFavoritesButtonClass))
#define GMPC_FAVORITES_IS_BUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GMPC_FAVORITES_TYPE_BUTTON))
#define GMPC_FAVORITES_IS_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GMPC_FAVORITES_TYPE_BUTTON))
#define GMPC_FAVORITES_BUTTON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GMPC_FAVORITES_TYPE_BUTTON, GmpcFavoritesButtonClass))

typedef struct _GmpcFavoritesButton GmpcFavoritesButton;
typedef struct _GmpcFavoritesButtonClass GmpcFavoritesButtonClass;
typedef struct _GmpcFavoritesButtonPrivate GmpcFavoritesButtonPrivate;

/**
     * This class is created, and stays active until the last GmpcFavoritesButton gets removed
     * POSSIBLE ISSUE: setting favorites list back to NULL seems to fail. It is no issue as 
     * I know atleast one will be active.
     */
struct _GmpcFavoritesList {
	GObject parent_instance;
	GmpcFavoritesListPrivate * priv;
};

struct _GmpcFavoritesListClass {
	GObjectClass parent_class;
};

struct _GmpcFavoritesListPrivate {
	MpdData* list;
};

/**
     * The actual favorite button
     */
struct _GmpcFavoritesButton {
	GtkEventBox parent_instance;
	GmpcFavoritesButtonPrivate * priv;
};

struct _GmpcFavoritesButtonClass {
	GtkEventBoxClass parent_class;
};

struct _GmpcFavoritesButtonPrivate {
	mpd_Song* song;
	GtkImage* image;
	gboolean fstate;
	GdkPixbuf* pb;
};



#define some_unique_name_fav VERSION
#define use_transition_fav TRUE
extern GmpcFavoritesList* favorites;
GmpcFavoritesList* favorites = NULL;
GType gmpc_favorites_list_get_type (void);
#define GMPC_FAVORITES_LIST_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GMPC_FAVORITES_TYPE_LIST, GmpcFavoritesListPrivate))
enum  {
	GMPC_FAVORITES_LIST_DUMMY_PROPERTY
};
static void gmpc_favorites_list_con_changed (GmpcFavoritesList* self, GmpcConnection* conn, MpdObj* server, gint connect);
static void gmpc_favorites_list_status_changed (GmpcFavoritesList* self, GmpcConnection* conn, MpdObj* server, ChangedStatusType what);
gboolean gmpc_favorites_list_is_favorite (GmpcFavoritesList* self, const char* path);
void gmpc_favorites_list_set_favorite (GmpcFavoritesList* self, const char* path, gboolean favorite);
GmpcFavoritesList* gmpc_favorites_list_new (void);
GmpcFavoritesList* gmpc_favorites_list_construct (GType object_type);
GmpcFavoritesList* gmpc_favorites_list_new (void);
static void _gmpc_favorites_list_con_changed_gmpc_connection_connection_changed (GmpcConnection* _sender, MpdObj* server, gint connect, gpointer self);
static void _gmpc_favorites_list_status_changed_gmpc_connection_status_changed (GmpcConnection* _sender, MpdObj* server, ChangedStatusType what, gpointer self);
static GObject * gmpc_favorites_list_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer gmpc_favorites_list_parent_class = NULL;
static void gmpc_favorites_list_finalize (GObject* obj);
GType gmpc_favorites_button_get_type (void);
#define GMPC_FAVORITES_BUTTON_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GMPC_FAVORITES_TYPE_BUTTON, GmpcFavoritesButtonPrivate))
enum  {
	GMPC_FAVORITES_BUTTON_DUMMY_PROPERTY
};
static gboolean gmpc_favorites_button_button_press_event_callback (GmpcFavoritesButton* self, GmpcFavoritesButton* button, const GdkEventButton* event);
static gboolean gmpc_favorites_button_enter_notify_event_callback (GmpcFavoritesButton* self, GmpcFavoritesButton* button, const GdkEventCrossing* motion);
static void gmpc_favorites_button_update (GmpcFavoritesButton* self, GmpcFavoritesList* list);
static gboolean gmpc_favorites_button_leave_notify_event_callback (GmpcFavoritesButton* self, GmpcFavoritesButton* button, const GdkEventCrossing* motion);
void gmpc_favorites_button_set_song (GmpcFavoritesButton* self, const mpd_Song* song);
GmpcFavoritesButton* gmpc_favorites_button_new (void);
GmpcFavoritesButton* gmpc_favorites_button_construct (GType object_type);
GmpcFavoritesButton* gmpc_favorites_button_new (void);
static void _gmpc_favorites_button_update_gmpc_favorites_list_updated (GmpcFavoritesList* _sender, gpointer self);
static gboolean _gmpc_favorites_button_button_press_event_callback_gtk_widget_button_press_event (GmpcFavoritesButton* _sender, const GdkEventButton* event, gpointer self);
static gboolean _gmpc_favorites_button_enter_notify_event_callback_gtk_widget_enter_notify_event (GmpcFavoritesButton* _sender, const GdkEventCrossing* event, gpointer self);
static gboolean _gmpc_favorites_button_leave_notify_event_callback_gtk_widget_leave_notify_event (GmpcFavoritesButton* _sender, const GdkEventCrossing* event, gpointer self);
static GObject * gmpc_favorites_button_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static gpointer gmpc_favorites_button_parent_class = NULL;
static void gmpc_favorites_button_finalize (GObject* obj);
static int _vala_strcmp0 (const char * str1, const char * str2);



/**
         * If disconnected from mpd, clear the list.
         * On connect fill 
         */
static void gmpc_favorites_list_con_changed (GmpcFavoritesList* self, GmpcConnection* conn, MpdObj* server, gint connect) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (conn != NULL);
	g_return_if_fail (server != NULL);
	if (connect == 1) {
		MpdData* _tmp0_;
		_tmp0_ = NULL;
		self->priv->list = (_tmp0_ = mpd_database_get_playlist_content (server, _ ("Favorites")), (self->priv->list == NULL) ? NULL : (self->priv->list = (mpd_data_free (self->priv->list), NULL)), _tmp0_);
		g_signal_emit_by_name (self, "updated");
	} else {
		MpdData* _tmp1_;
		_tmp1_ = NULL;
		self->priv->list = (_tmp1_ = NULL, (self->priv->list == NULL) ? NULL : (self->priv->list = (mpd_data_free (self->priv->list), NULL)), _tmp1_);
	}
}


/**
         * If playlist changed update the list
         */
static void gmpc_favorites_list_status_changed (GmpcFavoritesList* self, GmpcConnection* conn, MpdObj* server, ChangedStatusType what) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (conn != NULL);
	g_return_if_fail (server != NULL);
	if ((what & MPD_CST_STORED_PLAYLIST) == MPD_CST_STORED_PLAYLIST) {
		MpdData* _tmp0_;
		_tmp0_ = NULL;
		self->priv->list = (_tmp0_ = mpd_database_get_playlist_content (server, _ ("Favorites")), (self->priv->list == NULL) ? NULL : (self->priv->list = (mpd_data_free (self->priv->list), NULL)), _tmp0_);
		g_signal_emit_by_name (self, "updated");
	}
}


/**
         * Check if the song (specified by path) is favored
         */
gboolean gmpc_favorites_list_is_favorite (GmpcFavoritesList* self, const char* path) {
	const MpdData* iter;
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	iter = mpd_data_get_first (self->priv->list);
	while (iter != NULL) {
		if (iter->type == MPD_DATA_TYPE_SONG) {
			if (_vala_strcmp0 (iter->song->file, path) == 0) {
				return TRUE;
			}
		}
		iter = mpd_data_get_next_real (iter, FALSE);
	}
	return FALSE;
}


/**
         * Favor, or unfavor a song
         */
void gmpc_favorites_list_set_favorite (GmpcFavoritesList* self, const char* path, gboolean favorite) {
	gboolean current;
	g_return_if_fail (self != NULL);
	g_return_if_fail (path != NULL);
	current = gmpc_favorites_list_is_favorite (self, path);
	/* Do nothing if state does not change */
	if (current != favorite) {
		if (favorite) {
			/* Add it */
			mpd_database_playlist_list_add (connection, _ ("Favorites"), path);
		} else {
			const MpdData* iter;
			/* Remove it 
			 To be able to remove it we have to first lookup the position 
			 This needs libmpd 0.18.1 */
			iter = mpd_data_get_first (self->priv->list);
			while (iter != NULL) {
				if (iter->type == MPD_DATA_TYPE_SONG) {
					if (_vala_strcmp0 (iter->song->file, path) == 0) {
						mpd_database_playlist_list_delete (connection, _ ("Favorites"), iter->song->pos);
						return;
					}
				}
				iter = mpd_data_get_next_real (iter, FALSE);
			}
		}
	}
}


/**
     * This class is created, and stays active until the last GmpcFavoritesButton gets removed
     * POSSIBLE ISSUE: setting favorites list back to NULL seems to fail. It is no issue as 
     * I know atleast one will be active.
     */
GmpcFavoritesList* gmpc_favorites_list_construct (GType object_type) {
	GmpcFavoritesList * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


GmpcFavoritesList* gmpc_favorites_list_new (void) {
	return gmpc_favorites_list_construct (GMPC_FAVORITES_TYPE_LIST);
}


static void _gmpc_favorites_list_con_changed_gmpc_connection_connection_changed (GmpcConnection* _sender, MpdObj* server, gint connect, gpointer self) {
	gmpc_favorites_list_con_changed (self, _sender, server, connect);
}


static void _gmpc_favorites_list_status_changed_gmpc_connection_status_changed (GmpcConnection* _sender, MpdObj* server, ChangedStatusType what, gpointer self) {
	gmpc_favorites_list_status_changed (self, _sender, server, what);
}


static GObject * gmpc_favorites_list_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GmpcFavoritesListClass * klass;
	GObjectClass * parent_class;
	GmpcFavoritesList * self;
	klass = GMPC_FAVORITES_LIST_CLASS (g_type_class_peek (GMPC_FAVORITES_TYPE_LIST));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GMPC_FAVORITES_LIST (obj);
	{
		g_signal_connect_object (gmpcconn, "connection-changed", (GCallback) _gmpc_favorites_list_con_changed_gmpc_connection_connection_changed, self, 0);
		g_signal_connect_object (gmpcconn, "status-changed", (GCallback) _gmpc_favorites_list_status_changed_gmpc_connection_status_changed, self, 0);
	}
	return obj;
}


static void gmpc_favorites_list_class_init (GmpcFavoritesListClass * klass) {
	gmpc_favorites_list_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (GmpcFavoritesListPrivate));
	G_OBJECT_CLASS (klass)->constructor = gmpc_favorites_list_constructor;
	G_OBJECT_CLASS (klass)->finalize = gmpc_favorites_list_finalize;
	g_signal_new ("updated", GMPC_FAVORITES_TYPE_LIST, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void gmpc_favorites_list_instance_init (GmpcFavoritesList * self) {
	self->priv = GMPC_FAVORITES_LIST_GET_PRIVATE (self);
	self->priv->list = NULL;
}


static void gmpc_favorites_list_finalize (GObject* obj) {
	GmpcFavoritesList * self;
	self = GMPC_FAVORITES_LIST (obj);
	(self->priv->list == NULL) ? NULL : (self->priv->list = (mpd_data_free (self->priv->list), NULL));
	G_OBJECT_CLASS (gmpc_favorites_list_parent_class)->finalize (obj);
}


GType gmpc_favorites_list_get_type (void) {
	static GType gmpc_favorites_list_type_id = 0;
	if (gmpc_favorites_list_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GmpcFavoritesListClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) gmpc_favorites_list_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GmpcFavoritesList), 0, (GInstanceInitFunc) gmpc_favorites_list_instance_init, NULL };
		gmpc_favorites_list_type_id = g_type_register_static (G_TYPE_OBJECT, "GmpcFavoritesList", &g_define_type_info, 0);
	}
	return gmpc_favorites_list_type_id;
}


static gboolean gmpc_favorites_button_button_press_event_callback (GmpcFavoritesButton* self, GmpcFavoritesButton* button, const GdkEventButton* event) {
	gboolean _tmp0_;
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (button != NULL, FALSE);
	_tmp0_ = FALSE;
	if ((*event).button == 1) {
		_tmp0_ = self->priv->song != NULL;
	} else {
		_tmp0_ = FALSE;
	}
	if (_tmp0_) {
		gmpc_favorites_list_set_favorite (favorites, self->priv->song->file, !self->priv->fstate);
		self->priv->fstate = !self->priv->fstate;
	}
	return FALSE;
}


/* on mouse over, do some pre-highlighting */
static gboolean gmpc_favorites_button_enter_notify_event_callback (GmpcFavoritesButton* self, GmpcFavoritesButton* button, const GdkEventCrossing* motion) {
	GdkPixbuf* pb2;
	gboolean _tmp0_;
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (button != NULL, FALSE);
	pb2 = gdk_pixbuf_copy (self->priv->pb);
	if (self->priv->fstate) {
		colorshift_pixbuf (pb2, self->priv->pb, 10);
	} else {
		colorshift_pixbuf (pb2, self->priv->pb, -50);
	}
	gtk_image_set_from_pixbuf (self->priv->image, pb2);
	return (_tmp0_ = FALSE, (pb2 == NULL) ? NULL : (pb2 = (g_object_unref (pb2), NULL)), _tmp0_);
}


/* Reset default highlighting */
static gboolean gmpc_favorites_button_leave_notify_event_callback (GmpcFavoritesButton* self, GmpcFavoritesButton* button, const GdkEventCrossing* motion) {
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (button != NULL, FALSE);
	gmpc_favorites_button_update (self, favorites);
	return FALSE;
}


/* Update the icon according to state */
static void gmpc_favorites_button_update (GmpcFavoritesButton* self, GmpcFavoritesList* list) {
	GdkPixbuf* pb2;
	g_return_if_fail (self != NULL);
	g_return_if_fail (list != NULL);
	if (self->priv->song != NULL) {
		self->priv->fstate = gmpc_favorites_list_is_favorite (favorites, self->priv->song->file);
	} else {
		/* Hide the widget and do nothing */
		gtk_widget_hide ((GtkWidget*) self);
		return;
	}
	/* Copy the pixbuf */
	pb2 = gdk_pixbuf_copy (self->priv->pb);
	/* Depending on the state colorshift the pixbuf */
	if (self->priv->fstate) {
		colorshift_pixbuf (pb2, self->priv->pb, 30);
	} else {
		colorshift_pixbuf (pb2, self->priv->pb, -80);
	}
	gtk_image_set_from_pixbuf (self->priv->image, pb2);
	gtk_widget_show ((GtkWidget*) self->priv->image);
	gtk_widget_show ((GtkWidget*) self);
	(pb2 == NULL) ? NULL : (pb2 = (g_object_unref (pb2), NULL));
}


/**********************************************************************
         * Public api
         ********************************************************************
 Set the song the button should watch. or NULL to watch non */
void gmpc_favorites_button_set_song (GmpcFavoritesButton* self, const mpd_Song* song) {
	gboolean _tmp0_;
	gboolean _tmp1_;
	gboolean _tmp2_;
	mpd_Song* _tmp4_;
	const mpd_Song* _tmp3_;
	g_return_if_fail (self != NULL);
	_tmp0_ = FALSE;
	if (self->priv->song == NULL) {
		_tmp0_ = song == NULL;
	} else {
		_tmp0_ = FALSE;
	}
	if (_tmp0_) {
		return;
	}
	_tmp1_ = FALSE;
	_tmp2_ = FALSE;
	if (self->priv->song != NULL) {
		_tmp2_ = song != NULL;
	} else {
		_tmp2_ = FALSE;
	}
	if (_tmp2_) {
		_tmp1_ = _vala_strcmp0 (self->priv->song->file, song->file) == 0;
	} else {
		_tmp1_ = FALSE;
	}
	if (_tmp1_) {
		return;
	}
	_tmp4_ = NULL;
	_tmp3_ = NULL;
	self->priv->song = (_tmp4_ = (_tmp3_ = song, (_tmp3_ == NULL) ? NULL : mpd_songDup (_tmp3_)), (self->priv->song == NULL) ? NULL : (self->priv->song = (mpd_freeSong (self->priv->song), NULL)), _tmp4_);
	gmpc_favorites_button_update (self, favorites);
}


/**
     * The actual favorite button
     */
GmpcFavoritesButton* gmpc_favorites_button_construct (GType object_type) {
	GmpcFavoritesButton * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


GmpcFavoritesButton* gmpc_favorites_button_new (void) {
	return gmpc_favorites_button_construct (GMPC_FAVORITES_TYPE_BUTTON);
}


static void _gmpc_favorites_button_update_gmpc_favorites_list_updated (GmpcFavoritesList* _sender, gpointer self) {
	gmpc_favorites_button_update (self, _sender);
}


static gboolean _gmpc_favorites_button_button_press_event_callback_gtk_widget_button_press_event (GmpcFavoritesButton* _sender, const GdkEventButton* event, gpointer self) {
	return gmpc_favorites_button_button_press_event_callback (self, _sender, event);
}


static gboolean _gmpc_favorites_button_enter_notify_event_callback_gtk_widget_enter_notify_event (GmpcFavoritesButton* _sender, const GdkEventCrossing* event, gpointer self) {
	return gmpc_favorites_button_enter_notify_event_callback (self, _sender, event);
}


static gboolean _gmpc_favorites_button_leave_notify_event_callback_gtk_widget_leave_notify_event (GmpcFavoritesButton* _sender, const GdkEventCrossing* event, gpointer self) {
	return gmpc_favorites_button_leave_notify_event_callback (self, _sender, event);
}


/**
         * Creation of the object 
         */
static GObject * gmpc_favorites_button_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GmpcFavoritesButtonClass * klass;
	GObjectClass * parent_class;
	GmpcFavoritesButton * self;
	GError * _inner_error_;
	klass = GMPC_FAVORITES_BUTTON_CLASS (g_type_class_peek (GMPC_FAVORITES_TYPE_BUTTON));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GMPC_FAVORITES_BUTTON (obj);
	_inner_error_ = NULL;
	{
		GtkIconTheme* _tmp0_;
		GtkIconTheme* it;
		GtkImage* _tmp5_;
		gtk_widget_set_no_show_all ((GtkWidget*) self, TRUE);
		gtk_event_box_set_visible_window ((GtkEventBox*) self, FALSE);
		_tmp0_ = NULL;
		it = (_tmp0_ = gtk_icon_theme_get_default (), (_tmp0_ == NULL) ? NULL : g_object_ref (_tmp0_));
		{
			GdkPixbuf* _tmp1_;
			GdkPixbuf* _tmp3_;
			GdkPixbuf* _tmp2_;
			_tmp1_ = gtk_icon_theme_load_icon (it, "emblem-favorite", 24, 0, &_inner_error_);
			if (_inner_error_ != NULL) {
				goto __catch0_g_error;
				goto __finally0;
			}
			_tmp3_ = NULL;
			_tmp2_ = NULL;
			self->priv->pb = (_tmp3_ = (_tmp2_ = _tmp1_, (_tmp2_ == NULL) ? NULL : g_object_ref (_tmp2_)), (self->priv->pb == NULL) ? NULL : (self->priv->pb = (g_object_unref (self->priv->pb), NULL)), _tmp3_);
		}
		goto __finally0;
		__catch0_g_error:
		{
			GError * e;
			e = _inner_error_;
			_inner_error_ = NULL;
			{
				fprintf (stdout, "error: %s\n", e->message);
				(e == NULL) ? NULL : (e = (g_error_free (e), NULL));
			}
		}
		__finally0:
		if (_inner_error_ != NULL) {
			(it == NULL) ? NULL : (it = (g_object_unref (it), NULL));
			g_critical ("file %s: line %d: uncaught error: %s", __FILE__, __LINE__, _inner_error_->message);
			g_clear_error (&_inner_error_);
		}
		if (favorites == NULL) {
			GmpcFavoritesList* _tmp4_;
			_tmp4_ = NULL;
			favorites = (_tmp4_ = gmpc_favorites_list_new (), (favorites == NULL) ? NULL : (favorites = (g_object_unref (favorites), NULL)), _tmp4_);
		} else {
			g_object_ref ((GObject*) favorites);
		}
		g_signal_connect_object (favorites, "updated", (GCallback) _gmpc_favorites_button_update_gmpc_favorites_list_updated, self, 0);
		_tmp5_ = NULL;
		self->priv->image = (_tmp5_ = g_object_ref_sink ((GtkImage*) gtk_image_new ()), (self->priv->image == NULL) ? NULL : (self->priv->image = (g_object_unref (self->priv->image), NULL)), _tmp5_);
		gmpc_favorites_button_update (self, favorites);
		gtk_container_add ((GtkContainer*) self, (GtkWidget*) self->priv->image);
		g_signal_connect_object ((GtkWidget*) self, "button-press-event", (GCallback) _gmpc_favorites_button_button_press_event_callback_gtk_widget_button_press_event, self, 0);
		g_signal_connect_object ((GtkWidget*) self, "enter-notify-event", (GCallback) _gmpc_favorites_button_enter_notify_event_callback_gtk_widget_enter_notify_event, self, 0);
		g_signal_connect_object ((GtkWidget*) self, "leave-notify-event", (GCallback) _gmpc_favorites_button_leave_notify_event_callback_gtk_widget_leave_notify_event, self, 0);
		(it == NULL) ? NULL : (it = (g_object_unref (it), NULL));
	}
	return obj;
}


static void gmpc_favorites_button_class_init (GmpcFavoritesButtonClass * klass) {
	gmpc_favorites_button_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (GmpcFavoritesButtonPrivate));
	G_OBJECT_CLASS (klass)->constructor = gmpc_favorites_button_constructor;
	G_OBJECT_CLASS (klass)->finalize = gmpc_favorites_button_finalize;
}


static void gmpc_favorites_button_instance_init (GmpcFavoritesButton * self) {
	self->priv = GMPC_FAVORITES_BUTTON_GET_PRIVATE (self);
	self->priv->fstate = FALSE;
	self->priv->pb = NULL;
}


static void gmpc_favorites_button_finalize (GObject* obj) {
	GmpcFavoritesButton * self;
	self = GMPC_FAVORITES_BUTTON (obj);
	{
		if (favorites != NULL) {
			g_object_unref ((GObject*) favorites);
		}
	}
	(self->priv->song == NULL) ? NULL : (self->priv->song = (mpd_freeSong (self->priv->song), NULL));
	(self->priv->image == NULL) ? NULL : (self->priv->image = (g_object_unref (self->priv->image), NULL));
	(self->priv->pb == NULL) ? NULL : (self->priv->pb = (g_object_unref (self->priv->pb), NULL));
	G_OBJECT_CLASS (gmpc_favorites_button_parent_class)->finalize (obj);
}


GType gmpc_favorites_button_get_type (void) {
	static GType gmpc_favorites_button_type_id = 0;
	if (gmpc_favorites_button_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GmpcFavoritesButtonClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) gmpc_favorites_button_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GmpcFavoritesButton), 0, (GInstanceInitFunc) gmpc_favorites_button_instance_init, NULL };
		gmpc_favorites_button_type_id = g_type_register_static (GTK_TYPE_EVENT_BOX, "GmpcFavoritesButton", &g_define_type_info, 0);
	}
	return gmpc_favorites_button_type_id;
}


static int _vala_strcmp0 (const char * str1, const char * str2) {
	if (str1 == NULL) {
		return -(str1 != str2);
	}
	if (str2 == NULL) {
		return str1 != str2;
	}
	return strcmp (str1, str2);
}




