
#ifndef __GMPC_SONG_LINKS_H__
#define __GMPC_SONG_LINKS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libmpd/libmpdclient.h>
#include <libmpd/libmpd.h>

G_BEGIN_DECLS


#define GMPC_SONG_TYPE_LINKS (gmpc_song_links_get_type ())
#define GMPC_SONG_LINKS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GMPC_SONG_TYPE_LINKS, GmpcSongLinks))
#define GMPC_SONG_LINKS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GMPC_SONG_TYPE_LINKS, GmpcSongLinksClass))
#define GMPC_SONG_IS_LINKS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GMPC_SONG_TYPE_LINKS))
#define GMPC_SONG_IS_LINKS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GMPC_SONG_TYPE_LINKS))
#define GMPC_SONG_LINKS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GMPC_SONG_TYPE_LINKS, GmpcSongLinksClass))

typedef struct _GmpcSongLinks GmpcSongLinks;
typedef struct _GmpcSongLinksClass GmpcSongLinksClass;
typedef struct _GmpcSongLinksPrivate GmpcSongLinksPrivate;

#define GMPC_SONG_LINKS_TYPE_TYPE (gmpc_song_links_type_get_type ())

struct _GmpcSongLinks {
	GtkFrame parent_instance;
	GmpcSongLinksPrivate * priv;
};

struct _GmpcSongLinksClass {
	GtkFrameClass parent_class;
};

typedef enum  {
	GMPC_SONG_LINKS_TYPE_ARTIST,
	GMPC_SONG_LINKS_TYPE_ALBUM,
	GMPC_SONG_LINKS_TYPE_SONG
} GmpcSongLinksType;


GType gmpc_song_links_get_type (void);
GType gmpc_song_links_type_get_type (void);
GmpcSongLinks* gmpc_song_links_new (GmpcSongLinksType type, const mpd_Song* song);
GmpcSongLinks* gmpc_song_links_construct (GType object_type, GmpcSongLinksType type, const mpd_Song* song);


G_END_DECLS

#endif
