/*
 * Copyright (C) 2004-2006 Qball Cow <Qball@qballcow.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <glade/glade.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <config.h>
#include "sm.h"
#include "plugin.h"
#include "main.h"
#include "misc.h"
#ifdef DEBUG
/* Testing */
#include <sys/time.h>

struct timeval tv_old = {0,0};
#endif
/* as internall plugin */
#include "playlist3-tag-browser.h"
#include "playlist3-artist-browser.h"
#include "playlist3-current-playlist-browser.h"


#ifdef ENABLE_MMKEYS
#include "mm-keys.h"
#endif
extern GladeXML *pl3_xml;
extern GtkWidget *pl3_cp_tree;
void init_playlist_store ();
void connection_changed(MpdObj *mi, int connect, gpointer data);
void   GmpcStatusChangedCallback(MpdObj *mi, ChangedStatusType what, void *userdata);
extern int debug_level;
void error_callback(MpdObj *mi, int error_id, char *error_msg, gpointer data);
gmpcPlugin **plugins = NULL;
int num_plugins = 0;

/*
 * the xml fle pointer to the player window
 */

GladeXML *xml_error_window = NULL;
GladeXML *xml_password_window = NULL;
int update_interface ();

/*
 * the ID of the update timeout
 */
guint update_timeout = 0;
void init_stock_icons ();
/*
 * The Config object
 */
config_obj *config = NULL;
/*
 * The Connection object
 */
MpdObj *connection = NULL;


inline void tic()
{
#ifdef DEBUG
	if(tv_old.tv_sec){
		abort();
	}
	gettimeofday(&tv_old, NULL);
#endif
}
inline void tac(char *name)
{
#ifdef DEBUG
	struct timeval tv = tv_old;
	if(!tv_old.tv_sec){
		abort();            	
	}
	gettimeofday(&tv_old, NULL);
	printf("%40s: %llu usec\n", name,(guint64)(tv_old.tv_usec-tv.tv_usec));
	tv_old.tv_sec = 0;
#endif
}

char *gmpc_get_full_image_path(char *filename)
{
	gchar *path;
#ifdef WIN32
	gchar *packagedir;
	packagedir = g_win32_get_package_installation_directory("gmpc", NULL);
	debug_printf(DEBUG_INFO, "Got %s as package installation dir", packagedir);

	path = g_build_filename(packagedir, "data", "images", filename, NULL);

	/* From a certain version of GTK+ this g_free will be needed, but for now it will free
	 * a pointer which is returned on further calls to g_win32_get...
	 * This bug is fixed now (30-10-2005), so it will probably be in glib 2.6.7 and/or 2.8.4
	 */
#if CHECK_GLIB_VERSION(2,8,4)
	g_free(packagedir);
#endif

#else
	path = g_strdup_printf("%s/%s", PIXMAP_PATH, filename);
#endif
	return path;
}

char *gmpc_get_full_glade_path(char *filename)
{
	gchar *path;
#ifdef WIN32
	gchar *packagedir;
	packagedir = g_win32_get_package_installation_directory("gmpc", NULL);
	debug_printf(DEBUG_INFO, "Got %s as package installation dir", packagedir);

	path = g_build_filename(packagedir, "data", "glade", filename, NULL);

	/* From a certain version of GTK+ this g_free will be needed, but for now it will free
	 * a pointer which is returned on further calls to g_win32_get...
	 * This bug is fixed now (30-10-2005), so it will probably be in glib 2.6.7 and/or 2.8.4
	 */
#if CHECK_GLIB_VERSION(2,8,4)
	g_free(packagedir);
#endif

#else
	path = g_strdup_printf("%s/%s", GLADE_PATH, filename);
#endif
	return path;
}



int main (int argc, char **argv)
{
	int i;
	char *config_path = NULL;
#ifdef ENABLE_MMKEYS
	MmKeys *keys = NULL;
#endif
	gchar *url = NULL;
	tic();
	/* debug stuff */
	debug_level = DEBUG_ERROR;
	if(argc > 1)
	{
		int i;
		for(i = 1; i< argc; i++)
		{
			if(!strncasecmp(argv[i], "--debug-level=", 14))
			{
				debug_level = atoi(&argv[i][14]);
				debug_level = (debug_level < 0)? -1:((debug_level > DEBUG_INFO)? DEBUG_INFO:debug_level);
			}
			else if (!strncasecmp(argv[i], "--version", 9))
			{
				printf("Gnome Music Player Client\n");
				printf("Version: %s\n", VERSION);
				printf("Revision: %s\n",REVISION);
				exit(0);
			}
			else if (!strncasecmp(argv[i], "--config=", 9))
			{
				config_path = g_strdup(&argv[i][9]);

			}
		}

	}
	tac("Parsing options");



#ifdef ENABLE_NLS
	tic();
	debug_printf(DEBUG_INFO, "Setting NLS");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	tac("setting NLS");
#endif

	/*
	 * initialize gtk
	 */
	debug_printf(DEBUG_INFO, "Initializing gtk ");
	tic();
	gtk_init (&argc, &argv);
	tac("Gtk Init");

#ifdef ENABLE_SM
	tic();
	smc_connect(argc, argv);
	tac("Sm Connection");
#endif	

	/* initialize threading */
	debug_printf(DEBUG_INFO,"Initializing threading");
	tic();
	qthread_init();
	tac("Initialise threading");

	/*
	 * stock icons
	 */
	debug_printf(DEBUG_INFO, "Loading stock icons");
	tic();
	init_stock_icons ();
	tac("Loading stock icons");


	tic();
	/* Check for and create dir if availible */
	url = g_strdup_printf("%s/.gmpc/", g_get_home_dir());
	debug_printf(DEBUG_INFO, "Checking for %s existence",url);
	if(!g_file_test(url, G_FILE_TEST_EXISTS))
	{
		debug_printf(DEBUG_INFO, "Trying to create %s",url);
		if(mkdir(url,0700) < 0)
		{
			debug_printf(DEBUG_ERROR, "Failed to create: %s\n", url);
			show_error_message("Failed to create ~/.gmpc/.", TRUE);
			abort();
		}
	}
	else if (!g_file_test(url, G_FILE_TEST_IS_DIR))
	{
		debug_printf(DEBUG_ERROR, "%s isn't a directory.\n", url);
		debug_printf(DEBUG_ERROR, "Quitting.\n");
		show_error_message("~/.gmpc/ isn't a directory.", TRUE);
		abort();
	}
	else
	{
		debug_printf(DEBUG_INFO, "%s exist and is directory",url);
	}
	g_free(url);
	tac("Setting up user directories");


	tic();
	/** Add the internall plugins 
	*/
	/* this shows the connection preferences */
	plugin_add(&connection_plug,0);
	/* this the server preferences */
	plugin_add(&server_plug,0);
	/* this shows the playlist preferences */
	plugin_add(&playlist_plug,0);
	/* The new testing information plugin */
	plugin_add(&info_plugin,0);            	
	/* this shows the markup stuff */
	plugin_add(&tag_plug,0);
#ifdef ENABLE_TRAYICON
	/* the tray icon */
	plugin_add(&tray_icon_plug,0);
#endif
	plugin_add(&cover_art_plug,0);
	/* the about windows :D*/
/*	plugin_add(&about_plug,0);
 */
	tac("Adding internall plugins");

	tic();
	/* load dynamic plugins */
	url = g_strdup_printf("%s/%s",GLADE_PATH, "plugins");
	plugin_load_dir(url);
	g_free(url);
	tac("Loading global plugins");

	/* user space dynamic plugins */
	/* plugins */
	tic();
	url = g_strdup_printf("%s/.gmpc/plugins/",g_get_home_dir());
	if(g_file_test(url, G_FILE_TEST_IS_DIR))
	{
		plugin_load_dir(url);
	}
	else
	{
		mkdir(url, 0777);
	}
	g_free(url);
	tac("Loading user plugins");

	tic();
	/* OPEN CONFIG FILE */
	if(!config_path)
	{
		url = g_strdup_printf("%s/.gmpc/gmpc.cfg", g_get_home_dir());
	}
	else{
		url = config_path;
	}
	debug_printf(DEBUG_INFO, "Trying to open the config file: %s", url);
	config = cfg_open(url);


	/* test if config open  */
	if(config == NULL)
	{
		debug_printf(DEBUG_ERROR,"Failed to save/load configuration:\n%s\n",url);
		show_error_message("Failed to load the configuration system", TRUE);
		abort();
	}

	g_free(url);
	tac("Opening config");

	
	/* initialize the cover art */
	tic();
	cover_art_init();
	tac("Initialising cover art system");

	tic();
	/* Create connection object */
	connection = mpd_new_default();
	if(connection == NULL)
	{
		debug_printf(DEBUG_ERROR,"Failed to create connection obj\n");
		return 1;
	}

	/* New Signal */
	mpd_signal_connect_status_changed(connection, GmpcStatusChangedCallback, NULL);
	mpd_signal_connect_error(connection, error_callback, NULL);
	mpd_signal_connect_connection_changed(connection, connection_changed, NULL);
	tac("Creating mpd connection object");

	tic();
	/* time todo some initialisation of plugins */
	for(i=0; i< num_plugins && plugins[i] != NULL;i++)
	{
		if(plugins[i]->init)
		{
			plugins[i]->init();
		}
	}
	tac("Initializing plugins");

	/* create the store for the playlist */
	tic();
	init_playlist_store ();
	tac("Playlist store");
	tic();
	create_playlist3();
	tac("Creating playlist");

	/*
	 * create timeouts 
	 * get the status every 1/2 second should be enough, but it's configurable.
	 */
	gtk_timeout_add (cfg_get_single_value_as_int_with_default(config,
				"connection","mpd-update-speed",500),
			(GSourceFunc)update_mpd_status, NULL);
	update_timeout = gtk_timeout_add (5000,(GSourceFunc)update_interface, NULL);

	gtk_init_add((GSourceFunc)update_interface, NULL);

#ifdef ENABLE_MMKEYS
	/*
	 * Keys
	 */
	tic();
	keys = mmkeys_new();
	g_signal_connect(G_OBJECT(keys), "mm_playpause", G_CALLBACK(play_song), NULL);
	g_signal_connect(G_OBJECT(keys), "mm_next", G_CALLBACK(next_song), NULL);
	g_signal_connect(G_OBJECT(keys), "mm_prev", G_CALLBACK(prev_song), NULL);
	g_signal_connect(G_OBJECT(keys), "mm_stop", G_CALLBACK(stop_song), NULL);
	tac("Initialise mmkeys");
#endif
	/*
	 * run the main loop
	 */
	gtk_main ();
	/* cleaning up. */
	mpd_free(connection);
	cfg_close(config);
	g_object_unref(playlist);
	return 0;
}


void main_quit()
{
	pl3_current_playlist_destroy();
	/* so it saves the playlist pos */
	mpd_signal_connect_connection_changed(connection, NULL, NULL);
	if(mpd_check_connected(connection))
	{
		mpd_disconnect(connection);
	}

	gtk_main_quit();
}

int update_interface ()
{
	/* check if there is an connection.*/
	if (!mpd_check_connected(connection)){
		/* update the popup  */
		if (cfg_get_single_value_as_int_with_default(config,
					"connection",
					"autoconnect",
					0))
		{
			connect_to_mpd ();
		}
	}
	/* now start updating the rest */
	return TRUE;
}

void init_stock_icons ()
{
	GtkIconFactory *factory;
	GdkPixbuf *pb;
	GtkIconSet *set;
	char *path;
	factory = gtk_icon_factory_new ();

	/*
	 * add media-audiofile
	 */
	path = gmpc_get_full_image_path("media-audiofile.png");
	pb = gdk_pixbuf_new_from_file (path ,NULL);
	g_free(path);

	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "media-audiofile", set);
	g_object_unref (G_OBJECT (pb));
	/*
	 * add media-stream
	 */
	path = gmpc_get_full_image_path("media-stream.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);

	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "media-stream", set);
	g_object_unref (G_OBJECT (pb));
	/*
	 * add media-artist
	 */
	path = gmpc_get_full_image_path("media-artist.png");
	pb = gdk_pixbuf_new_from_file (path,NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "media-artist", set);
	g_object_unref (G_OBJECT (pb));
	/*
	 * add media-album
	 */
	path = gmpc_get_full_image_path("media-album.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "media-album", set);
	g_object_unref (G_OBJECT (pb));

	/*
	 * add player-shuffle
	 */
	path = gmpc_get_full_image_path("player-shuffle.png");
	pb = gdk_pixbuf_new_from_file (path,NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "media-random", set);
	g_object_unref (G_OBJECT (pb));
	/*
	 * add playerrepeat
	 */
	path = gmpc_get_full_image_path("player-repeat.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "media-repeat", set);
	g_object_unref (G_OBJECT (pb));

	/*
	 * add icecast
	 */
	path = gmpc_get_full_image_path("icecast.png");
	pb = gdk_pixbuf_new_from_file (path,NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "icecast", set);
	g_object_unref (G_OBJECT (pb));

	/*
	 * add media playlist
	 */
	path = gmpc_get_full_image_path("media-playlist.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "media-playlist", set);
	g_object_unref (G_OBJECT (pb));


	/*
	 * add media playlist
	 */
	path = gmpc_get_full_image_path("gmpc.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "gmpc", set);
	g_object_unref (G_OBJECT (pb));



	/*
	 * add media playlist
	 */
	path = gmpc_get_full_image_path("gmpc-tray.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "gmpc-tray", set);
	g_object_unref (G_OBJECT (pb));

	path = gmpc_get_full_image_path("gmpc-tray-play.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "gmpc-tray-play", set);
	g_object_unref (G_OBJECT (pb));

	path = gmpc_get_full_image_path("gmpc-tray-pause.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "gmpc-tray-pause", set);
	g_object_unref (G_OBJECT (pb));


	path = gmpc_get_full_image_path("gmpc-tray-disconnected.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);
	gtk_icon_factory_add (factory, "gmpc-tray-disconnected", set);
	g_object_unref (G_OBJECT (pb));

	path = gmpc_get_full_image_path("gmpc-no-cover.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);              	
	gtk_icon_factory_add (factory, "media-no-cover", set);
	g_object_unref (G_OBJECT (pb));


	path = gmpc_get_full_image_path("stock_volume.png");
	pb = gdk_pixbuf_new_from_file (path, NULL);
	g_free(path);
	set = gtk_icon_set_new_from_pixbuf (pb);              	
	gtk_icon_factory_add (factory, "gmpc-volume", set);
	g_object_unref (G_OBJECT (pb));



	gtk_icon_factory_add_default (factory);
}


void init_playlist_store ()
{

	playlist = (GtkTreeModel *)playlist_list_new();

	playlist_list_set_markup((CustomList *)playlist,cfg_get_single_value_as_string_with_default(config,
				"playlist","markup", DEFAULT_PLAYLIST_MARKUP));


}


void   GmpcStatusChangedCallback(MpdObj *mi, ChangedStatusType what, void *userdata)
{
	int i;
	if(what&MPD_CST_SONGPOS)
	{
		playlist_list_set_current_song_pos(PLAYLIST_LIST(playlist), mpd_player_get_current_song_pos(mi));
	}
	if(what&MPD_CST_SONGID)
	{
		pl3_highlight_song_change();
	}
	if(what&MPD_CST_DATABASE)
	{
		pl3_database_changed();
	}
	if(what&MPD_CST_UPDATING)
	{
		pl3_updating_changed(connection, mpd_status_db_is_updating(connection));
	}
	if(what&MPD_CST_STATE)
	{
		if(mpd_player_get_state(mi) == MPD_STATUS_STATE_STOP){
			playlist_list_set_current_song_pos(PLAYLIST_LIST(playlist), -1);
		}
		else if (mpd_player_get_state(mi) == MPD_STATUS_STATE_PLAY) {
			playlist_list_set_current_song_pos(PLAYLIST_LIST(playlist), mpd_player_get_current_song_pos(mi));
		}
	}
	if(what&MPD_CST_PLAYLIST)
	{
		playlist_list_data_update(PLAYLIST_LIST(playlist),mi,GTK_TREE_VIEW(pl3_cp_tree));
	}

	/* make the player handle signals */
	//player_mpd_state_changed(mi,what,userdata);
	id3_status_update();

	for(i=0; i< num_plugins; i++)
	{
		if(plugins[i]->mpd_status_changed!= NULL)
		{
			plugins[i]->mpd_status_changed(mi,what,NULL);
		}
	}
}

void error_window_destroy(GtkWidget *window,int response, gpointer autoconnect)
{

	gtk_widget_destroy(window);
	g_object_unref(xml_error_window);
	xml_error_window = NULL;
	if(response == GTK_RESPONSE_OK)
	{
		cfg_set_single_value_as_int(config, "connection", "autoconnect", GPOINTER_TO_INT(autoconnect));
		connect_to_mpd();
	}
}

void password_dialog(int failed)
{
	gchar *path  = NULL;
	if(xml_password_window) return;
	path = gmpc_get_full_glade_path("gmpc.glade");
	xml_password_window = glade_xml_new(path, "password-dialog",NULL);
	g_free(path);
	if(!xml_password_window) return;
	if(failed)
	{
		path = g_strdup_printf("Failed to set password on: '%s'\nPlease try again",
				cfg_get_single_value_as_string(config, "connection", "hostname"));
	}
	else
	{
		path = g_strdup_printf("Please enter your password for: '%s'",
				cfg_get_single_value_as_string(config, "connection", "hostname"));

	}
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml_password_window, "pass_label")),path);
	g_free(path);
	switch(gtk_dialog_run(GTK_DIALOG(glade_xml_get_widget(xml_password_window, "password-dialog"))))
	{
		case GTK_RESPONSE_OK:
			{
				path = (char *)gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml_password_window, "pass_entry")));
				mpd_set_password(connection, path);
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(xml_password_window, "ck_save_pass"))))
				{
					cfg_set_single_value_as_int(config, "connection", "useauth", TRUE);
					cfg_set_single_value_as_string(config, "connection", "password", path);
				}
				mpd_send_password(connection);
			}
			break;
		default:
			cfg_set_single_value_as_int(config, "connection", "autoconnect", FALSE);
			mpd_disconnect(connection);
			break;


	}
	gtk_widget_destroy(glade_xml_get_widget(xml_password_window, "password-dialog"));
	g_object_unref(xml_password_window);
	xml_password_window = NULL;
}

void error_callback(MpdObj *mi, int error_id, char *error_msg, gpointer data)
{
	int autoconnect = cfg_get_single_value_as_int_with_default(config, "connection","autoconnect", DEFAULT_AUTOCONNECT);
	/* if we are not connected we show a reconnect */
	if(!mpd_check_connected(mi))
	{
		/* no response? then we just ignore it when autoconnecting. */
		if(error_id == 15 && autoconnect) return;
		cfg_set_single_value_as_int(config, "connection", "autoconnect", 0);
		if (xml_error_window == NULL)
		{
			gchar *str = g_strdup_printf("error code %i: %s", error_id, error_msg);
			gchar *path = gmpc_get_full_glade_path("gmpc.glade");
			xml_error_window = glade_xml_new(path,"error_dialog",NULL);
			g_free(path);
			GtkWidget *dialog = glade_xml_get_widget(xml_error_window, "error_dialog");
			gtk_label_set_markup(GTK_LABEL(glade_xml_get_widget(xml_error_window,"em_label")), str);
			gtk_widget_show_all(dialog);
			g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(error_window_destroy), GINT_TO_POINTER(autoconnect));
			g_free(str);
		}
		else
		{
			gchar *str = g_strdup_printf("error code %i: %s", error_id, error_msg);
			gtk_label_set_markup(GTK_LABEL(glade_xml_get_widget(xml_error_window,"em_label")), str);
			g_free(str);
		}
	}
	else
	{
		if(error_id == MPD_ACK_ERROR_PASSWORD)
		{
			password_dialog(TRUE);
		}
		else if (error_id == MPD_ACK_ERROR_PERMISSION)
		{
			password_dialog(FALSE);
		}
		else {

			GtkWidget *dialog = gtk_message_dialog_new_with_markup(NULL,
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					_("Mpd Returned the following error:\n<i>\"%s\"</i>"),
					error_msg);

			g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_destroy), NULL);
			gtk_widget_show_all(dialog);
		}
	}
}

void connect_callback(MpdObj *mi)
{
	if(xml_error_window != NULL)
	{
		int autocon = cfg_get_single_value_as_int_with_default(config,
				"connection",
				"autoconnect",
				DEFAULT_AUTOCONNECT);
		error_window_destroy(glade_xml_get_widget(xml_error_window, "error_dialog"),0,
				GINT_TO_POINTER(autocon));
	}
	gtk_timeout_remove (update_timeout);
	update_timeout = gtk_timeout_add (400,(GSourceFunc)update_interface, NULL);
}


void connection_changed(MpdObj *mi, int connect, gpointer data)
{
	int i=0;
	/* send password, first thing we do */
	if(cfg_get_single_value_as_int_with_default(config, "connection", "useauth",0))
	{
		mpd_send_password(connection);
	}                                                                              	
	debug_printf(DEBUG_INFO, "Connection changed\n");
	playlist_connection_changed(mi, connect);
	tray_icon_connection_changed(mi, connect);
	for(i=0; i< num_plugins; i++)
	{
		debug_printf(DEBUG_INFO, "Connection changed plugin: %s\n", plugins[i]->name);
		if(plugins[i]->mpd_connection_changed!= NULL)
		{
			plugins[i]->mpd_connection_changed(mi,connect,NULL);
		}
	}
	mpd_status_update(mi);
}

void show_error_message(gchar *string, int block)
{
	GtkWidget *dialog = NULL;
	dialog = gtk_message_dialog_new_with_markup(NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			string);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_destroy), NULL);
	if(block)
	{
		gtk_dialog_run(GTK_DIALOG(dialog));
	}
	else
	{
		gtk_widget_show(dialog);
	}
}
