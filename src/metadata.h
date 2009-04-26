/* Gnome Music Player Client (GMPC)
 * Copyright (C) 2004-2009 Qball Cow <qball@sarine.nl>
 * Project homepage: http://gmpc.wikia.com/
 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __METADATA_H__
#define __METADATA_H__



typedef enum {
	META_ALBUM_ART 			= 1,		/* Album Cover art 	*/
	META_ARTIST_ART 		= 2,		/* Artist  image 	*/
	META_ALBUM_TXT 			= 4,		/* Album story 		*/
	META_ARTIST_TXT 		= 8, 		/* Artist biography 	*/
	META_SONG_TXT			= 16,		/* Lyrics 		*/
	META_ARTIST_SIMILAR 	= 32,		/* Similar artists */
	META_SONG_SIMILAR       = 64,       /* Similar songs */
	META_QUERY_DATA_TYPES  	= 127, 		/* Bitmask for getting the metadata types only */
	META_QUERY_NO_CACHE 	= 128		/* Do the query withouth checking the cache first */
}MetaDataType;

typedef enum {
	META_DATA_AVAILABLE,
	META_DATA_UNAVAILABLE,
	META_DATA_FETCHING
} MetaDataResult;

typedef void (*MetaDataCallback)(mpd_Song *song, MetaDataResult result, char *path, gpointer data);

/**
 * The MetaAata object that will be used in the future to pass metadata around
 */

/**
 * This can be extended 
 */
typedef enum {
    META_DATA_CONTENT_EMPTY,
    /* contains a string with an uri to the data */
    META_DATA_CONTENT_URI,
    /* Contains result (text) */
    META_DATA_CONTENT_TEXT,
    /* Contains result (raw image data) */
    META_DATA_CONTENT_RAW,
    /* Contains result (text) in html format */
    META_DATA_CONTENT_HTML,
    /* A null-terminated array of strings, f.e. similar artists*/
    META_DATA_CONTENT_TEXT_VECTOR
} MetaDataContentType;

typedef struct {
    /* The MetaDataType this holds */
    MetaDataType type;
    /* The name of the plugin that provided the data 
     * This can be NULL if unknown.
     **/
    const gchar *plugin_name;
    /* The data type */
    MetaDataContentType content_type;
    /* The contents */
    void  *content;
    /* size is only used for raw data. Might be set for text, but that needs to be 
     * null terminated anyway.
     */
    gsize size;
}MetaData;
/**
 * Create empty MetaData
 */
MetaData * meta_data_new(void);
/**
 * Free MetaData object
 */
void meta_data_free(MetaData *data);
/**
 * copy a MetaData object
 */
MetaData *meta_data_dup(MetaData *data);
/**
 * Steals the data from the orginal.
 * This can be used to avoid having large data blocks copied.
 **/
MetaData *meta_data_dup_steal(MetaData *data);

const gchar *meta_data_get_uri(const MetaData *data);
const gchar * meta_data_get_text(const MetaData *data);
const gchar * meta_data_get_html(const MetaData *data);
const guchar * meta_data_get_raw(const MetaData *data, gsize *length);
const gchar **meta_data_get_text_vector(const MetaData *data);

/* ****************************************** */
void metadata_import_old_db(char *url);

#include "gmpc-meta-watcher.h"
extern GmpcMetaWatcher *gmw;

/*guint meta_data_get_path_callback(mpd_Song *song, MetaDataType type, MetaDataCallback callback, gpointer data);*/

void meta_data_init(void);
void meta_data_check_plugin_changed(void);
void meta_data_handle_remove_request(guint id);
void meta_data_destroy(void);

MetaDataResult meta_data_get_path(mpd_Song *tsong, MetaDataType type, gchar **path,MetaDataCallback callback, gpointer data);


#define METADATA_DIR ".covers"
gchar * gmpc_get_metadata_filename(MetaDataType  type, mpd_Song *song, char *extention);



gpointer metadata_get_list(mpd_Song  *song, MetaDataType type, void (*callback)(gpointer handle, const gchar *plugin_name, GList *list, gpointer data), gpointer data);

void metadata_get_list_cancel(gpointer data);

#endif
