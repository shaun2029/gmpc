#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>
#include <libmpd/libmpd.h>
#include <string.h>

#include "plugin.h"

#include "gmpc-mpddata-model.h"
#include "gmpc-mpddata-treeview.h"
#include "gmpc-mpddata-model-sort.h"
#include "browsers/playlist3-playlist-editor.h"
#include "main.h"

#define DEFAULT_MARKUP_BROWSER 	"[%name%: &[%artist% - ]%title%]|%name%|[%artist% - ]%title%|%shortfile%|"

static GtkTreeRowReference *playlist_editor_browser_ref = NULL;
static GtkWidget *playlist_editor_browser = NULL;
static GtkWidget *playlist_editor_song_tree = NULL;
static void playlist_editor_status_changed(MpdObj *mi, ChangedStatusType what, void *data);
static int playlist_editor_add_go_menu(GtkWidget *);
GtkWidget *playlist_editor_icon_view = NULL;
GmpcMpdDataModelSort *playlist_editor_list_store = NULL;

static void playlist_editor_save_myself(void);

enum {
	PL_NAME,
    PL_MTIME,
	PL_IMAGE,
	PL_NUM_COLS
};

GtkListStore *playlist_editor_store = NULL;

/**
 * Enable/Disable plugin
 */
static int playlist_editor_get_enabled(void)
{
	return cfg_get_single_value_as_int_with_default(config, "playlist-plugin", "enable", TRUE);
}
static void playlist_editor_set_enabled(int enabled)
{
	cfg_set_single_value_as_int(config, "playlist-plugin", "enable", enabled);
};


/**
 * Browser extention 
 */
gmpcPlBrowserPlugin playlist_editor_gbp = {
  .add = playlist_editor_browser_add,   /* Add */
  .selected = playlist_editor_browser_selected,   /* Selected */
  .unselected = playlist_editor_browser_unselected,   /* Unselected */
  .cat_right_mouse_menu = playlist_editor_browser_cat_menu,   /* */
  .add_go_menu = playlist_editor_add_go_menu
};

gmpcPlugin playlist_editor_plugin = {
  .name 					= "Playlist Editor",
  .version					=  {0,15,0},
  .plugin_type 				= GMPC_PLUGIN_PL_BROWSER,
  .init						= playlist_editor_init,
  .destroy 					= playlist_editor_destroy, 
  .browser					=  &playlist_editor_gbp,
  .mpd_status_changed		= playlist_editor_status_changed,
  .mpd_connection_changed 	= playlist_editor_conn_changed,
  .get_enabled				= playlist_editor_get_enabled,
  .set_enabled				= playlist_editor_set_enabled,
  .save_yourself			= playlist_editor_save_myself
};

/**
 * Init plugin
 */
void playlist_editor_init(void)
{
}
/**
 * Destroy Plugin
 */
void playlist_editor_destroy(void)
{
  if(playlist_editor_browser)
  {
    gtk_widget_destroy(playlist_editor_browser);
    playlist_editor_browser = NULL;
  }
}


/**
 * Connection changed
 */
void playlist_editor_conn_changed(MpdObj *mi, int connect, void *userdata)
{
	playlist_editor_fill_list();
}


void playlist_editor_browser_add(GtkWidget *cat_tree)
{
	GtkListStore *store= playlist3_get_category_tree_store();
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	gint pos = cfg_get_single_value_as_int_with_default(config, "playlist-plugin","position",6);

	/* Check if enabled */
	if(!cfg_get_single_value_as_int_with_default(config, "playlist-plugin", "enable", TRUE)) return;

	playlist3_insert_browser(&iter, pos);
	gtk_list_store_set(store, &iter, 
			PL3_CAT_TYPE, playlist_editor_plugin.id,
			PL3_CAT_TITLE, _("Playlist Editor"),
			PL3_CAT_INT_ID, "",
			PL3_CAT_ICON_ID, "media-playlist",
			PL3_CAT_PROC, TRUE,
			PL3_CAT_ICON_SIZE,GTK_ICON_SIZE_DND,-1);
	/**
	 * Clean up old row reference if it exists
	 */
	if (playlist_editor_browser_ref)
	{
		gtk_tree_row_reference_free(playlist_editor_browser_ref);
		playlist_editor_browser_ref = NULL;
	}
	/**
	 * create row reference
	 */
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(playlist3_get_category_tree_store()), &iter);
	if (path)
	{
		playlist_editor_browser_ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(playlist3_get_category_tree_store()), path);
		gtk_tree_path_free(path);
	}


}

static void playlist_editor_browser_playlist_editor_selected(GtkIconView *giv, GtkTreePath *path, gpointer userdata)
{
	gchar *pl_path = NULL;
	GtkTreeIter iter;

	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_editor_store), &iter, path))
	{
		MpdData *data ;
		gtk_tree_model_get(GTK_TREE_MODEL(playlist_editor_store), &iter, PL_NAME, &pl_path, -1);
		data = mpd_database_get_playlist_content(connection, pl_path); 
		gmpc_mpddata_model_set_mpd_data(GMPC_MPDDATA_MODEL(playlist_editor_list_store), data);		
		gmpc_mpddata_model_sort_set_playlist(GMPC_MPDDATA_MODEL_SORT(playlist_editor_list_store), pl_path);
		g_free(pl_path);
	}
}
static void playlist_editor_browser_playlist_editor_changed(GtkWidget *giv, gpointer data)
{

	gmpc_mpddata_model_set_mpd_data(GMPC_MPDDATA_MODEL(playlist_editor_list_store), NULL);		
	gmpc_mpddata_model_sort_set_playlist(GMPC_MPDDATA_MODEL_SORT(playlist_editor_list_store), NULL);
	/* iter all the selected items (aka 1) */
	gtk_icon_view_selected_foreach(GTK_ICON_VIEW(giv), playlist_editor_browser_playlist_editor_selected, NULL);
}

static gint __sort_func(gpointer aa, gpointer bb, gpointer c)
{
    MpdData_real *a = *(MpdData_real **)aa;
    MpdData_real *b = *(MpdData_real **)bb;
    if(a->type == MPD_DATA_TYPE_PLAYLIST && b->type == MPD_DATA_TYPE_PLAYLIST)
    {
        if(a->playlist->path == NULL && b->playlist->path != NULL)
            return -1;
        else if(b->playlist->path == NULL && a->playlist->path != NULL)
            return 1;
        else if (a->playlist->path  && b->playlist->path)
        {
            int val;
            gchar *sa,*sb;
            sa = g_utf8_strdown(a->playlist->path, -1);
            sb = g_utf8_strdown(b->playlist->path, -1);
            val = g_utf8_collate(sa,sb);
            g_free(sa);
            g_free(sb);
            return val;
        }
    }
    return a->type - b->type;
}

void playlist_editor_fill_list(void)
{
    GtkTreeIter iter;
    if(playlist_editor_browser)
	{
        gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playlist_editor_store), &iter);
//        int selected = 0;
		MpdData *data = mpd_database_playlist_list(connection);
        data = misc_sort_mpddata(data, (GCompareDataFunc) __sort_func, NULL);
//		gtk_list_store_clear(playlist_editor_store);
		for(;data;data =mpd_data_get_next(data))
		{
			if(data->type ==  MPD_DATA_TYPE_PLAYLIST)
			{
loop:            
                if(valid) {
                    char *name = NULL;
                    int val = 0;
                    gtk_tree_model_get(GTK_TREE_MODEL(playlist_editor_store), &iter, PL_NAME, &name, -1);
                    val = strcmp(name,data->playlist->path);

                    if(val == 0) { 
                        GtkTreePath *path = NULL;
                        gchar *mtime = NULL;
                        gtk_tree_model_get(GTK_TREE_MODEL(playlist_editor_store), &iter, PL_MTIME, &mtime, -1);
                        gtk_list_store_set(playlist_editor_store, &iter, PL_MTIME, data->playlist->mtime, -1);
                        printf("compare %s-%s\n", name,data->playlist->path);
                        /* do nothing */
                        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(playlist_editor_store), &iter);
                        path = gtk_tree_model_get_path(GTK_TREE_MODEL(playlist_editor_store), &iter);
                        if(path) {
                            printf("path\n");
                            if(gtk_icon_view_path_is_selected(GTK_ICON_VIEW(playlist_editor_icon_view), path))
                            {
                                printf("is selected\n");
                                if(mtime == NULL || data->playlist->mtime == NULL || strcmp(mtime, data->playlist->mtime) == 0) {
                                    printf("playlist updated\n");
                                    /* update view */
                                    playlist_editor_browser_playlist_editor_changed(playlist_editor_icon_view, NULL);
                                }
                            }
                            gtk_tree_path_free(path);
                        }
                        g_free(mtime);
                    }
                    else if (val > 0) {
                        GtkTreeIter niter;
                        GdkPixbuf *pb = NULL; 
                        printf("add-before %s-%s\n", name,data->playlist->path);
                        pb = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), "media-playlist", 64, 0,NULL);
                        gtk_list_store_insert_before(playlist_editor_store,&niter, &iter);
                        gtk_list_store_set(playlist_editor_store, &niter,PL_NAME, data->playlist->path, PL_MTIME, data->playlist->mtime, PL_IMAGE, pb, -1);
                        if(pb)
                            g_object_unref(pb);
                    }else{
                        printf("remove: %s\n", name);
                    /*
                        GtkTreeIter niter;
                        GdkPixbuf *pb = NULL; 
                        pb = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), "media-playlist", 64, 0,NULL);
                        gtk_list_store_insert_after(playlist_editor_store,&niter, &iter);
                        gtk_list_store_set(playlist_editor_store, &niter,PL_NAME, data->playlist->path, PL_MTIME, data->playlist->mtime, PL_IMAGE, pb, -1);
                        if(pb)
                            g_object_unref(pb);
                        iter = niter;
                        */
                        valid = gtk_list_store_remove(playlist_editor_store, &iter);
                        g_free(name);
                        goto loop;
                    }
                    g_free(name);

                }
                else
                {
                    GdkPixbuf *pb = NULL; 
                    pb = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), "media-playlist", 64, 0,NULL);
                    gtk_list_store_append(playlist_editor_store, &iter);
                    gtk_list_store_set(playlist_editor_store, &iter,PL_NAME, data->playlist->path, PL_MTIME, data->playlist->mtime, PL_IMAGE, pb, -1);
                    if(pb)
                        g_object_unref(pb);
                        /*
                    if(!selected)
                    {
                        GtkTreePath *path;
                        path = gtk_tree_model_get_path(GTK_TREE_MODEL(playlist_editor_store), &iter);
                        if(path)
                        {
                            gtk_icon_view_select_path(GTK_ICON_VIEW(playlist_editor_icon_view), path);
                            gtk_tree_path_free(path);
                            selected = TRUE;
                        }
                    }
                    */
                }
			}
		}
	}
}
static void playlist_editor_browser_activate_cursor_item(GtkIconView *giv, gpointer userdata)
{
	gchar *pl_path = NULL;
	GtkTreeIter iter;
	GList *it,*list = gtk_icon_view_get_selected_items(giv);
	for(it = g_list_first(list);it; it = g_list_next(it))
	{
		GtkTreePath *path = it->data;
		if(gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_editor_store), &iter, path))
		{
			gtk_tree_model_get(GTK_TREE_MODEL(playlist_editor_store), &iter, PL_NAME, &pl_path, -1);
			mpd_playlist_queue_load(connection, pl_path);
			g_free(pl_path);
		}
	}
	mpd_playlist_queue_commit(connection);
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);
}
/*********
 * Playlist list handling 
 */
static void playlist_editor_list_delete_songs(GtkButton *button, GtkTreeView *tree) 
{
	gchar *pl_path = NULL;
	GList *it,*list = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(playlist_editor_icon_view));
	for(it = g_list_first(list);it; it = g_list_next(it))
	{
		GtkTreePath *path = it->data;
		GtkTreeIter iter;
		if(gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_editor_store), &iter, path) && !pl_path)
		{
			gtk_tree_model_get(GTK_TREE_MODEL(playlist_editor_store), &iter, PL_NAME, &pl_path, -1);
		}
	}
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);

	if(pl_path)
	{
//		MpdData *data2 = NULL;
		GtkTreeSelection *sel = gtk_tree_view_get_selection(tree);
		GtkTreeModel *model = gtk_tree_view_get_model(tree);
		GList *data;
		list= gtk_tree_selection_get_selected_rows(sel, &model);
		for(data = g_list_last(list); data; data = g_list_previous(data))
		{
			GtkTreePath *path = data->data;
			GtkTreeIter iter;
			if(gtk_tree_model_get_iter(model, &iter, path))
			{
				int *pos = gtk_tree_path_get_indices(path);
				mpd_database_playlist_list_delete(connection, pl_path,pos[0]);
			}
		}
        debug_printf(DEBUG_INFO,"ping\n");
		g_list_foreach (list,(GFunc) gtk_tree_path_free, NULL);
		g_list_free (list);

	/*	
		data2 = mpd_database_get_playlist_content(connection, pl_path); 
		gmpc_mpddata_model_set_mpd_data(GMPC_MPDDATA_MODEL(playlist_editor_list_store), data2);		
		gmpc_mpddata_model_sort_set_playlist(GMPC_MPDDATA_MODEL_SORT(playlist_editor_list_store), pl_path);
*/
		g_free(pl_path);
	}
}


static void playlist_editor_list_add_songs(GtkButton *button, GtkTreeView *tree) 
{
	GtkTreeSelection *sel= gtk_tree_view_get_selection(tree);
	GtkTreeModel *model = gtk_tree_view_get_model(tree);
	GList *data,*list= gtk_tree_selection_get_selected_rows(sel, &model);
	for(data = g_list_first(list); data; data = g_list_next(data))
	{
		GtkTreePath *path = data->data;
		GtkTreeIter iter;
		if(gtk_tree_model_get_iter(model, &iter, path))
		{
			gchar *song_path;
			gtk_tree_model_get(model, &iter,MPDDATA_MODEL_COL_PATH, &song_path, -1); 
			mpd_playlist_queue_add(connection, song_path);

			g_free(song_path);
		}
	}
	mpd_playlist_queue_commit(connection);
	g_list_foreach (list,(GFunc) gtk_tree_path_free, NULL);
	g_list_free (list);
}
static void playlist_editor_clear_playlist(GtkWidget *item, gpointer data)
{
  gchar *path = g_object_get_data(G_OBJECT(item), "path");
  mpd_database_playlist_clear(connection,path);
  playlist_editor_browser_playlist_editor_changed(playlist_editor_icon_view,NULL);

}
static void playlist_editor_load_playlist(GtkWidget *item, gpointer data)
{
  gchar *path = g_object_get_data(G_OBJECT(item), "path");
  mpd_playlist_queue_load(connection,path);
  mpd_playlist_queue_commit(connection);
}
static void playlist_editor_replace_playlist(GtkWidget *item, gpointer data)
{
  gchar *path = g_object_get_data(G_OBJECT(item), "path");
  mpd_playlist_clear(connection);
  mpd_playlist_queue_load(connection,path);
  mpd_playlist_queue_commit(connection);
  mpd_player_play(connection);
}
static void playlist_editor_delete_playlist(GtkWidget *item, gpointer data)
{
  gchar *path = g_object_get_data(G_OBJECT(item), "path");
  mpd_database_delete_playlist(connection, path);
  playlist_editor_fill_list();
}

static void playlist_editor_new_entry_changed(GtkEntry *entry, GtkWidget *button)
{
	if(strlen(gtk_entry_get_text(entry)) > 0)
	{
		/* todo check existing */
		gtk_widget_set_sensitive(button,TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(button,FALSE);
	}
}
static void playlist_editor_new_playlist(GtkWidget *item, gpointer data)
{
	int done = 0;
    GtkWidget *pl3_win = glade_xml_get_widget(pl3_xml, "pl3_win");
    GtkWidget *dialog = gtk_dialog_new_with_buttons(_("New playlist"), NULL,GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				NULL);
	GtkWidget *button = NULL; 
	GtkWidget *hbox = gtk_hbox_new(FALSE,6);
	GtkWidget *label = gtk_label_new(_("Name:"));	
	GtkWidget *entry = gtk_entry_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(pl3_win));
	button = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_NEW, GTK_RESPONSE_ACCEPT);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(playlist_editor_new_entry_changed), button);
	gtk_widget_set_sensitive(button, FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE,0);
	
	gtk_misc_set_alignment(GTK_MISC(label), 1,0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 9);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 3);
	gtk_widget_show_all(dialog);
	while(!done)
	{
		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			case GTK_RESPONSE_ACCEPT:
                g_object_set_data_full(G_OBJECT(item), "playlist", g_strdup(gtk_entry_get_text(GTK_ENTRY(entry))), g_free);
                mpd_database_playlist_clear(connection, gtk_entry_get_text(GTK_ENTRY(entry)));
				playlist_editor_fill_list();

			default:
				done = TRUE;
				break;
		}
	}
	gtk_widget_destroy(dialog);
}
static void playlist_editor_rename_playlist(GtkWidget *item, gpointer data)
{
	int done = 0;
	char *name = g_object_get_data(G_OBJECT(item), "path");
	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Rename Playlist"), NULL,GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				NULL);
	GtkWidget *button = NULL; 
	GtkWidget *hbox = gtk_hbox_new(FALSE,6);
	GtkWidget *label = gtk_label_new(_("Name:"));	
	GtkWidget *entry = gtk_entry_new();
	button = gtk_dialog_add_button(GTK_DIALOG(dialog), _("Rename"), GTK_RESPONSE_ACCEPT);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(playlist_editor_new_entry_changed), button);
	gtk_entry_set_text(GTK_ENTRY(entry), name);
	gtk_widget_set_sensitive(button, FALSE);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE,0);
	
	gtk_misc_set_alignment(GTK_MISC(label), 1,0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 9);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 3);

	gtk_widget_show_all(dialog);
	while(!done)
	{
		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			case GTK_RESPONSE_ACCEPT:
				mpd_database_playlist_rename(connection, name,gtk_entry_get_text(GTK_ENTRY(entry)));
				playlist_editor_fill_list();
			default:
				done = TRUE;
				break;
		}
	}
	gtk_widget_destroy(dialog);
}

static void playlist_editor_edit_columns(void)
{
  gmpc_mpddata_treeview_edit_columns(GMPC_MPDDATA_TREEVIEW(playlist_editor_song_tree));
}

static gboolean playlist_editor_key_released(GtkTreeView *tree, GdkEventButton *button, gpointer data)
{
	if(button->button == 3)
	{
		GtkTreeSelection *sel= gtk_tree_view_get_selection(tree);


			GtkWidget *menu = gtk_menu_new();
      GtkWidget *item = NULL;
      if(gtk_tree_selection_count_selected_rows(sel) > 0)
      {
        item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ADD,NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_editor_list_add_songs), tree);
      	if(mpd_server_check_version(connection, 0,13,0))
        {
          item = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE,NULL);
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
          g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_editor_list_delete_songs), tree);
        }
      }
      item = gtk_image_menu_item_new_with_label(_("Edit Columns"));
      gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item),
          gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU));
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
      g_signal_connect(G_OBJECT(item), "activate",
          G_CALLBACK(playlist_editor_edit_columns), NULL);




      gtk_widget_show_all(menu);
      gtk_menu_popup(GTK_MENU(menu), NULL, NULL,NULL, NULL, 0, button->time);
      return TRUE;
  }
  return FALSE;
}
static gboolean playlist_editor_key_pressed(GtkWidget *giv, GdkEventButton *event, gpointer data)
{
	GtkTreePath *path = NULL;
	if(event->button == 3 &&gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(giv), event->x, event->y,&path,NULL,NULL,NULL))
	{	
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(giv));
		if(gtk_tree_selection_path_is_selected(sel, path))
		{
			gtk_tree_path_free(path);
			return TRUE;
		}
	}
	if(path) {
		gtk_tree_path_free(path);
	}
	return FALSE;                                                                                                     
}
static gboolean playlist_editor_browser_button_release_event(GtkWidget *giv, GdkEventButton *event, gpointer data)
{
  if(event->button == 3)
  {
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item = NULL;
    GList *list = NULL;
    /* New */
    if(mpd_server_check_version(connection, 0,13,0))
    {
      item = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW,NULL);
      gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
      g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_editor_new_playlist),NULL);                                   
    }

    list = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(playlist_editor_icon_view));
    if(list)
    {
      char *path = NULL;
      GtkTreeIter iter;
      if(gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist_editor_store), &iter, (GtkTreePath *)list->data))
      {
        gtk_tree_model_get(GTK_TREE_MODEL(playlist_editor_store), &iter,PL_NAME, &path, -1); 

        /* replace */
        item = gtk_image_menu_item_new_with_label(_("Replace"));
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
        g_object_set_data_full(G_OBJECT(item), "path", g_strdup(path), g_free);
        gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), item);                            
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_editor_replace_playlist),NULL);                                   
        /* load */
        item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ADD,NULL);
        g_object_set_data_full(G_OBJECT(item), "path", g_strdup(path), g_free);
        gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), item);                            
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_editor_load_playlist),NULL);

        /* delete */
        item = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE,NULL);
        g_object_set_data_full(G_OBJECT(item), "path", g_strdup(path), g_free);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);     
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_editor_delete_playlist),NULL);

        if(mpd_server_check_version(connection, 0,13,0))
        {
            /* delete */
            item = gtk_image_menu_item_new_with_label(_("Rename"));
            gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), gtk_image_new_from_stock(GTK_STOCK_EDIT,GTK_ICON_SIZE_MENU));
            g_object_set_data_full(G_OBJECT(item), "path", g_strdup(path), g_free);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);     
            g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_editor_rename_playlist),NULL);

            /* clear */
            item = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLEAR,NULL);
            g_object_set_data_full(G_OBJECT(item), "path", g_strdup(path), g_free);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);     
            g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(playlist_editor_clear_playlist),NULL);
        }

        g_free(path);
      }
      g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
      g_list_free (list);
    }

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL,NULL, NULL, 0, event->time);
    return TRUE;
  }
  return FALSE;
}
static void playtime_changed(GmpcMpdDataModel *model, gulong playtime)
{
    if(pl3_cat_get_selected_browser() == playlist_editor_plugin.id)
    {
        playlist3_show_playtime(playtime);
    }
}


static void playlist_editor_browser_init()
{
    GtkWidget *tree = NULL;
    GtkWidget *sw = NULL;
    /* */
    playlist_editor_browser = gtk_vpaned_new();
    /** browser */
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_size_request(sw, -1, 100);	
    gtk_paned_add1(GTK_PANED(playlist_editor_browser), sw);

    /* icon view*/
    playlist_editor_store = gtk_list_store_new(PL_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING,GDK_TYPE_PIXBUF);

    playlist_editor_icon_view = tree = gtk_icon_view_new_with_model(GTK_TREE_MODEL(playlist_editor_store));
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(tree), GTK_SELECTION_BROWSE);
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(tree), PL_IMAGE);
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(tree), PL_NAME);
    gtk_container_add(GTK_CONTAINER(sw), tree);

    g_signal_connect(G_OBJECT(tree), "selection-changed", G_CALLBACK(playlist_editor_browser_playlist_editor_changed), NULL);
    g_signal_connect(G_OBJECT(tree), "item-activated", G_CALLBACK(playlist_editor_browser_activate_cursor_item), NULL);
    g_signal_connect(G_OBJECT(tree), "button-release-event", G_CALLBACK(playlist_editor_browser_button_release_event), NULL);

    /* file list */

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);

    //	gtk_box_pack_start(GTK_BOX(playlist_editor_browser), sw, TRUE, TRUE,0);
    gtk_paned_add2(GTK_PANED(playlist_editor_browser), sw);


    playlist_editor_list_store= gmpc_mpddata_model_sort_new();
    gmpc_mpddata_model_disable_image(GMPC_MPDDATA_MODEL(playlist_editor_list_store));
    g_signal_connect(G_OBJECT(playlist_editor_list_store), "playtime_changed", G_CALLBACK(playtime_changed), NULL);


    playlist_editor_song_tree = tree = gmpc_mpddata_treeview_new("playlist-browser",FALSE, GTK_TREE_MODEL(playlist_editor_list_store));
    gtk_container_add(GTK_CONTAINER(sw), tree);
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(tree), TRUE);

    g_signal_connect(G_OBJECT(tree), "button-release-event", G_CALLBACK(playlist_editor_key_released), NULL);
    g_signal_connect(G_OBJECT(tree), "button-press-event", G_CALLBACK(playlist_editor_key_pressed), NULL);
    g_object_ref(playlist_editor_browser);

    gtk_widget_show_all(playlist_editor_browser);
}

void playlist_editor_browser_selected(GtkWidget *container)
{
    if(!playlist_editor_browser) {
        playlist_editor_browser_init();
        playlist_editor_fill_list();
    }
    gtk_container_add(GTK_CONTAINER(container), playlist_editor_browser);
    gtk_widget_show_all(playlist_editor_browser);

    playlist3_show_playtime(gmpc_mpddata_model_get_playtime(GMPC_MPDDATA_MODEL(playlist_editor_list_store))); 
}

void playlist_editor_browser_unselected(GtkWidget *container)
{
    gtk_container_remove(GTK_CONTAINER(container), playlist_editor_browser);
}

int playlist_editor_browser_cat_menu(GtkWidget *menu, int type, GtkWidget *tree, GdkEventButton *event)
{
    if(type == playlist_editor_plugin.id)
    {
    }
    return 0;
}

static void playlist_editor_add_to_new(GtkWidget *item, gpointer data)
{
    void (*callback)(GtkWidget *item, gpointer data) = data;
    playlist_editor_new_playlist(item, NULL);
    callback(item, NULL); 


}

void playlist_editor_right_mouse(GtkWidget *menu, void (*add_to_playlist)(GtkWidget *menu))
{
    GtkWidget *sitem;
    GtkWidget *item;
    GtkWidget *smenu;

    if(!mpd_server_check_version(connection, 0,13,0))
    {
        return;
    }



    smenu  = gtk_menu_new();

    sitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, NULL);
    g_signal_connect(G_OBJECT(sitem), "activate", G_CALLBACK(playlist_editor_add_to_new), add_to_playlist);
    gtk_menu_shell_append(GTK_MENU_SHELL(smenu), sitem);

    {
        MpdData *data = mpd_database_playlist_list(connection);
        for(;data;data =mpd_data_get_next(data))
        {
            if(data->type ==  MPD_DATA_TYPE_PLAYLIST)
            {
                sitem = gtk_image_menu_item_new_with_label(data->playlist->path);
                gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(sitem), 
                        gtk_image_new_from_icon_name("media-playlist", GTK_ICON_SIZE_MENU));
                g_object_set_data_full(G_OBJECT(sitem),"playlist", g_strdup(data->playlist->path), g_free);
                gtk_menu_shell_append(GTK_MENU_SHELL(smenu), sitem);
                g_signal_connect(G_OBJECT(sitem), "activate", G_CALLBACK(add_to_playlist), NULL);
                gtk_widget_show(sitem);                             				
            }
        }
    }

    /* Add */
    item = gtk_menu_item_new_with_label(_("Add to playlist"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), smenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_widget_show(item);

    gtk_widget_show(smenu);
}

static void playlist_editor_status_changed(MpdObj *mi, ChangedStatusType what, void *data)
{
    if(what&MPD_CST_STORED_PLAYLIST)
    {
        printf("Failed list\n");
        playlist_editor_fill_list();
    }
}
static void playlist_editor_activate(GtkWidget *item, gpointer data)
{
    GtkTreeSelection *selec = gtk_tree_view_get_selection((GtkTreeView *)
            playlist3_get_category_tree_view());	

    if(playlist_editor_browser_ref)
    {
        GtkTreePath *path = gtk_tree_row_reference_get_path(playlist_editor_browser_ref); 
        if(path)
        {
            gtk_tree_selection_select_path(selec, path);
            gtk_tree_path_free(path);
        }
    }
}


static int playlist_editor_add_go_menu(GtkWidget *menu)
{
    GtkWidget *item = NULL;
    if(!cfg_get_single_value_as_int_with_default(config, "playlist-plugin", "enable", 1)) return 0;
    item = gtk_image_menu_item_new_with_label(_("Playlist Editor"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), 
            gtk_image_new_from_icon_name("media-playlist", GTK_ICON_SIZE_MENU));

    gtk_widget_add_accelerator(GTK_WIDGET(item), 
            "activate", 
            gtk_menu_get_accel_group(GTK_MENU(menu)), 
            GDK_F5, 0, 
            GTK_ACCEL_VISIBLE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    g_signal_connect(G_OBJECT(item), "activate", 
            G_CALLBACK(playlist_editor_activate), NULL);
    return 1;
}

static void playlist_editor_save_myself(void)
{
    if(playlist_editor_browser_ref)
    {
        GtkTreePath *path = gtk_tree_row_reference_get_path(playlist_editor_browser_ref);
        if(path)
        {
            gint *indices = gtk_tree_path_get_indices(path);
            debug_printf(DEBUG_INFO,"Saving myself to position: %i\n", indices[0]);
            cfg_set_single_value_as_int(config, "playlist-plugin","position",indices[0]);
            gtk_tree_path_free(path);
        }
    }
}

