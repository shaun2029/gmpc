#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "main.h"
#include "metadata.h"
#include "config1.h"
#include "metadata_cache.h"

#define CACHE_NAME "Metadata cache"
/* TODO: Move all to this file */
config_obj *cover_index;

/**
 * Checking the cache
 * !!NEEDS TO BE THREAD SAFE !!
 */
MetaDataResult meta_data_get_from_cache(mpd_Song *song, MetaDataType type, MetaData **met)
{
    (*met) = meta_data_new();
    (*met)->type = type;
    (*met)->content_type = META_DATA_CONTENT_EMPTY;
    (*met)->plugin_name = CACHE_NAME;
    if(!song)
    {
		return META_DATA_UNAVAILABLE;	
	}
	/* Get values acording to type */
	if(type == META_ALBUM_ART)
	{
        gchar *path = NULL;
		if(!song->artist || !song->album)
		{
			return META_DATA_UNAVAILABLE;	
		}
		path = cfg_get_single_value_as_string_mm(cover_index,song->artist,song->album, NULL, "image");
		if(path)
		{
			/* if path length is NULL, then data unavailible */
			if(strlen(path) == 0)
			{
				q_free(path);
				return META_DATA_UNAVAILABLE;	
			}
			/* return that data is availible */
			if(!g_file_test(path, G_FILE_TEST_EXISTS))
			{
				cfg_del_single_value_mm(cover_index, song->artist,song->album, NULL, "image");
				q_free(path);
				return META_DATA_FETCHING;	
			}
            (*met)->content_type = META_DATA_CONTENT_URI;
            (*met)->content =path; (*met)->size = -1;
            return META_DATA_AVAILABLE;
		}

		/* else default to fetching */
	}
	/* Get values acording to type */
	else if(type == META_ARTIST_SIMILAR)
	{
        gchar *path = NULL;
		if(!song->artist)
		{
			return META_DATA_UNAVAILABLE;	
		}
		path = cfg_get_single_value_as_string_mm(cover_index,song->artist,NULL, NULL,  "similar_artist");
		if(path)
		{
			/* if path length is NULL, then data unavailible */
			if(strlen(path) == 0)
			{
				q_free(path);
				return META_DATA_UNAVAILABLE;	
			}
            (*met)->content_type = META_DATA_CONTENT_RAW;
            (*met)->content = path; (*met)->size = -1;
			return META_DATA_AVAILABLE;
		}

		/* else default to fetching */
	}

	else if(type == META_ALBUM_TXT)
	{
		gchar *path = NULL;
		if(!song->artist || !song->album)
		{
			return META_DATA_UNAVAILABLE;	
		}
		path = cfg_get_single_value_as_string_mm(cover_index,song->artist, song->album, NULL, "info");
		if(path)
		{
			/* if path length is NULL, then data unavailible */
			if(strlen(path) == 0)
			{
				q_free(path);                                                  		
				path = NULL;
				return META_DATA_UNAVAILABLE;	
			}
			/* return that data is availible */
			if(!g_file_test(path, G_FILE_TEST_EXISTS))
			{
				cfg_del_single_value_mm(cover_index, song->artist,song->album, NULL, "info");
				q_free(path);
				path = NULL;
				return META_DATA_FETCHING;	
			}
            (*met)->content_type = META_DATA_CONTENT_URI;
            (*met)->content = path; (*met)->size = -1;
			/* return that data is availible */
			return META_DATA_AVAILABLE;
		}
	}
	else if (type == META_ARTIST_ART)
	{
		gchar *path = NULL;
		if(!song->artist)
		{
			return META_DATA_UNAVAILABLE;	
		}
		path = cfg_get_single_value_as_string_mm(cover_index,song->artist,NULL, NULL, "image");
		if(path)
		{
			/* if path length is NULL, then data unavailible */
			if(strlen(path) == 0)
			{
				q_free(path);                                                  		
				return META_DATA_UNAVAILABLE;	
			}
			/* return that data is availible */
			if(!g_file_test(path, G_FILE_TEST_EXISTS))
			{
				cfg_del_single_value_mm(cover_index, song->artist,NULL, NULL,"image");
				q_free(path);
				return META_DATA_FETCHING;	
			}

            (*met)->content_type = META_DATA_CONTENT_URI;
            (*met)->content = path; (*met)->size = -1;
			/* return that data is availible */
			return META_DATA_AVAILABLE;
		}
	}
	else if (type == META_ARTIST_TXT)
	{
        gchar *path = NULL;
        if(!song->artist)
		{
			return META_DATA_UNAVAILABLE;	
		}
		path = cfg_get_single_value_as_string_mm(cover_index,song->artist,  NULL, NULL, "biography");
		if(path)
		{
			/* if path length is NULL, then data unavailible */
			if(strlen(path) == 0)
			{
				q_free(path);                                                  		
				return META_DATA_UNAVAILABLE;	
			}
			/* return that data is availible */
			if(!g_file_test(path, G_FILE_TEST_EXISTS))
			{
				cfg_del_single_value_mm(cover_index, song->artist, NULL, NULL,"biography");
				q_free(path);
				return META_DATA_FETCHING;	
			}

            (*met)->content_type = META_DATA_CONTENT_URI;
            (*met)->content = path; (*met)->size = -1;
			/* return that data is availible */
			return META_DATA_AVAILABLE;
		}

	}
	else if(type == META_SONG_TXT)
	{
		gchar *path = NULL;
		if(!song->artist || !song->title)
		{
			return META_DATA_UNAVAILABLE;	
		}
		path = cfg_get_single_value_as_string_mm(cover_index,song->artist,"lyrics",NULL, song->title);
		if(path)
		{
			/* if path length is NULL, then data unavailible */
			if(strlen(path) == 0)
			{
				q_free(path);                                                  		
				return META_DATA_UNAVAILABLE;	
			}
			/* return that data is availible */
			if(!g_file_test(path, G_FILE_TEST_EXISTS))
			{
				cfg_del_single_value_mm(cover_index, song->artist,"lyrics", NULL, song->title);
				q_free(path);
				return META_DATA_FETCHING;	
			}

            (*met)->content_type = META_DATA_CONTENT_URI;
            (*met)->content = path; (*met)->size = -1;
			/* return that data is availible */
			return META_DATA_AVAILABLE;
		}	
	}
	else if(type == META_SONG_SIMILAR)
	{
        char *path = NULL;
		if(!song->artist && song->title)
		{
			return META_DATA_UNAVAILABLE;	
		}

		path = cfg_get_single_value_as_string_mm(cover_index,song->artist,"similar_song", NULL, song->title);
		if(path)
		{
			/* if path length is NULL, then data unavailible */
			if(strlen(path) == 0)
			{
				q_free(path);
				return META_DATA_UNAVAILABLE;	
			}

            (*met)->content_type = META_DATA_CONTENT_RAW;
            (*met)->content = path; (*met)->size = -1;
			return META_DATA_AVAILABLE;
		}

		/* else default to fetching */
	}
	return META_DATA_FETCHING;	
}

void meta_data_set_cache_real(mpd_Song *song, MetaDataType type, MetaDataResult result, char *path)
{
	if(!song) return;
	/**
	 * Save the path for the album art
	 */
	if(type == META_ALBUM_ART) {
		if(song->artist && song->album) {
//			char *temp = g_strdup_printf("album:%s", song->album);
			if(result == META_DATA_AVAILABLE) {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,song->album,NULL, "image",path);
			} else {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,song->album,NULL, "image","");
			}
//			q_free(temp);
		}
	} else if(type == META_ALBUM_TXT) {
		if(song->artist && song->album)	{
//			char *temp = g_strdup_printf("albumtxt:%s", song->album);                   		
			if(result == META_DATA_AVAILABLE) {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,song->album, NULL, "info",path);
			} else {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,song->album, NULL, "info","");
			}                                                                        		
//			q_free(temp);
		}
	} else if (type == META_ARTIST_ART) {
		if(song->artist) {
			if(result == META_DATA_AVAILABLE) {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,NULL, NULL, "image",path);
			} else {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,NULL, NULL, "image","");
			}                                                                        		
		}
	} else if (type == META_ARTIST_TXT) {
		if(song->artist) {
			if(result == META_DATA_AVAILABLE) {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,NULL, NULL, "biography",path);
			} else {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,NULL, NULL, "biography","");
			}                                                                        		
		}
	} else if (type == META_ARTIST_SIMILAR) {
		if(song->artist) {
			if(result == META_DATA_AVAILABLE) {
				cfg_set_single_value_as_string_mm(cover_index, song->artist, NULL, NULL,"similar_artist",path);
			} else {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,NULL, NULL, "similar_artist","");
			}                                                                        		
		}
	} else if (type == META_SONG_TXT)	{
		if(song->artist && song->title) {
			if(result == META_DATA_AVAILABLE) {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,"lyrics", NULL, song->title,path);
			} else {
				cfg_set_single_value_as_string_mm(cover_index, song->artist, "lyrics", NULL, song->title,"");
			}                                                                        		
		}
	}else if (type == META_SONG_SIMILAR) {
		if(song->artist && song->title) {
			if(result == META_DATA_AVAILABLE) {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,"similar_song",NULL, song->title,path);
			} else {
				cfg_set_single_value_as_string_mm(cover_index, song->artist,"similar_song",NULL, song->title,"");
			}                                                                        		
		}
	}

}
void meta_data_set_cache(mpd_Song *song, MetaDataType type, MetaDataResult result, char *path)
{
    mpd_Song *edited = rewrite_mpd_song(song, type);
    meta_data_set_cache_real(edited, type, result, path);
    if(edited->artist)
    {
        if(strcmp(edited->artist, "Various Artists")!=0)
            meta_data_set_cache_real(song, type, result, path);
    }
    
    mpd_freeSong(edited);
}

void metadata_import_old_db(char *url)
{
    config_obj *old = cfg_open(url);
    if(old)
    {
        conf_mult_obj *mult, *iter;

        mult = cfg_get_class_list(old);
        for(iter = mult; iter; iter = iter->next)
        {
            char *artist = iter->key;
            if(artist)
            {
                mpd_Song *song = mpd_newSong();
                conf_mult_obj *cm , *citer;
                cm = cfg_get_key_list(old, artist);
                song->artist = artist;
                for(citer = cm; citer; citer = citer->next)
                {
                    /* Artist only */
                    if(strcmp(citer->key, "image") == 0)
                    {
                        meta_data_set_cache_real(song, 
                                META_ARTIST_ART, 
                                (strlen(citer->value) > 0)?META_DATA_AVAILABLE:META_DATA_UNAVAILABLE, citer->value);  
                    }else if (strcmp(citer->key, "similar") == 0)
                    {
                        meta_data_set_cache_real(song, 
                                META_ARTIST_SIMILAR, 
                                (strlen(citer->value) > 0)?META_DATA_AVAILABLE:META_DATA_UNAVAILABLE, citer->value);  
                    }else if (strcmp(citer->key, "biography") == 0)
                    {
                        meta_data_set_cache_real(song, 
                                META_ARTIST_TXT, 
                                (strlen(citer->value) > 0)?META_DATA_AVAILABLE:META_DATA_UNAVAILABLE, citer->value);  
                    }
                    /* Album */
                    else if(strncmp(citer->key, "album:",6) == 0)
                    {
                        song->album = &((citer->key)[6]);
                        meta_data_set_cache_real(song, 
                                META_ALBUM_ART, 
                                (strlen(citer->value) > 0)?META_DATA_AVAILABLE:META_DATA_UNAVAILABLE, citer->value);  
                    }
                    else if (strncmp(citer->key, "albumtxt:", 9) == 0)
                    {
                        song->album = &((citer->key)[9]);
                        meta_data_set_cache_real(song, 
                                META_ALBUM_TXT, 
                                (strlen(citer->value) > 0)?META_DATA_AVAILABLE:META_DATA_UNAVAILABLE, citer->value);  
                    }
                    /* song */
                    else if (strncmp(citer->key, "lyrics:",7) == 0) 
                    {
                        song->title = &((citer->key)[7]);
                        meta_data_set_cache_real(song, 
                                META_SONG_TXT,
                                (strlen(citer->value) > 0)?META_DATA_AVAILABLE:META_DATA_UNAVAILABLE, citer->value);  
                    }else if (strncmp(citer->key, "similar:", 8) == 0)
                    {
                        song->title = &((citer->key)[8]);
                        meta_data_set_cache_real(song, 
                                META_SONG_SIMILAR,
                                (strlen(citer->value) > 0)?META_DATA_AVAILABLE:META_DATA_UNAVAILABLE, citer->value);  
                    }

                    song->album = NULL;
                    song->title = NULL;
                }
                song->artist = NULL;
                mpd_freeSong(song);
                cfg_free_multiple(cm);
            }
        }
        if(mult)
            cfg_free_multiple(mult);
        cfg_close(old); 
    }
}

void metadata_cache_init(void)
{
    gchar *url = gmpc_get_covers_path(NULL);
    if(!g_file_test(url,G_FILE_TEST_IS_DIR)){
        if(g_mkdir(url, 0700)<0){
            g_error("Cannot make %s\n", url);
        }
    }
    q_free(url);
    url = gmpc_get_covers_path("covers.db2");
    cover_index = cfg_open(url);
    q_free(url);

}


void metadata_cache_cleanup(void)
{
    cfg_do_special_cleanup(cover_index);
}
void metadata_cache_destroy(void)
{
    cfg_close(cover_index);
}
