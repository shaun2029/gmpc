#include <gtk/gtk.h>
#include <glade/glade.h>
#include <string.h>
#include <time.h>
#include "libmpdclient.h"
#include "main.h"
#include "config1.h"
extern config_obj *config;

enum
{
        ENABLED_COL,
	NAME_COL,
        ID_COL,
        N_COLUMNS
};



extern int last_db;
GladeXML *xml_preferences_window;
gboolean running = 0, connected = 0;
void popup_timeout_changed();
void tray_enable_toggled(GtkToggleButton *but);
void preferences_update();
void popup_enable_toggled(GtkToggleButton *but);
void popup_position_changed(GtkOptionMenu *om);
void update_auth_settings();
void preferences_window_connect(GtkWidget *but);
void preferences_window_disconnect(GtkWidget *but);
void update_popup_settings();
void update_tray_settings();
void update_server_settings();
void auth_enable_toggled(GtkToggleButton *but);
void entry_auth_changed(GtkEntry *entry);
void xfade_time_changed(GtkSpinButton *but);
void xfade_enable_toggled(GtkToggleButton *bug);
void set_display_settings();
void update_display_settings();
void create_outputs_tree();
void outputs_toggled(GtkCellRendererToggle *cell, gchar *path_str, GtkTreeView *view);
void update_outputs_settings();

void create_preferences_window()
	{
	GtkWidget *dialog;
	if(running)
	{
		if(xml_preferences_window == NULL)
		{
			running = 0;
		} 
		else
		{
			dialog = glade_xml_get_widget(xml_preferences_window, "preferences_window");
			gtk_window_present(GTK_WINDOW(dialog));
			return;
		}
	}
	xml_preferences_window = glade_xml_new(GLADE_PATH"gmpc.glade", "preferences_window", NULL);
	/* check for errors and axit when there is no gui file */
	if(xml_preferences_window == NULL)  g_error("Couldnt initialize GUI. Please check installation\n");


	/* set info from struct */
	/* hostname */
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "hostname_entry")), 
			cfg_get_single_value_as_string_with_default(config, "connection","hostname","localhost"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "port_spin")), 
			cfg_get_single_value_as_int_with_default(config, "connection","portnumber",6600));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "timeout_spin")), 
			cfg_get_single_value_as_float_with_default(config,"connection", "timeout",1.0));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(xml_preferences_window, "ck_autocon")), 
			cfg_get_single_value_as_int_with_default(config,"connection", "autoconnect", 0));
	create_outputs_tree();
	dialog = glade_xml_get_widget(xml_preferences_window, "preferences_window");
	gtk_widget_show_all(GTK_WIDGET(dialog));
	running = 1;

	update_server_settings();

	/* set the right sensitive stuff */
	if(info.connection == NULL)
	{
		gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "bt_con"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "bt_dis"), FALSE);	    
	}
	else
	{
		gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "bt_con"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "bt_dis"), TRUE);	    
	}
	update_popup_settings();
	update_tray_settings();
	update_auth_settings();

	if(info.stats != NULL)
	{
		gchar *buffer = ctime(&info.stats->dbUpdateTime);
		/* nasty but I need to get rid of the trailing new line */
		buffer[strlen(buffer)-1]='\0';
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml_preferences_window, "db_lu")),buffer);
	}

	set_display_settings();


	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(xml_preferences_window, "osb_ck_enable")), 
			cfg_get_single_value_as_int_with_default(config,"osb", "enable", 0));           	

	glade_xml_signal_autoconnect(xml_preferences_window);	

	}

void update_display_settings()
{
	gchar *temp;
	temp = g_strcompress(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "en_sd"))));
	cfg_set_single_value_as_string(config, "player","display_markup",temp);
	g_free(temp);
	temp = g_strcompress(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "en_pl"))));
	cfg_set_single_value_as_string(config, "playlist", "markup", temp);
	g_free(temp);
}

void set_display_settings()
{
	char *escaped = g_strescape(cfg_get_single_value_as_string_with_default(config, "player", "display_markup",
				"[%name%: &[%artist% - ]%title%]|%name%|[%artist% - ]%title%|%shortfile%|"),"");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "en_sd")), escaped);
	g_free(escaped);
	escaped = g_strescape(cfg_get_single_value_as_string_with_default(config, "playlist", "markup", DEFAULT_PLAYLIST_MARKUP), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "en_pl")), escaped);
	g_free(escaped);
}

void set_display_default_sd()
{
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "en_sd")), DEFAULT_PLAYER_MARKUP);
}
void set_display_default_pl()
{
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "en_pl")), DEFAULT_PLAYLIST_MARKUP);
}
void set_display_default_sb()
{
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "en_sb")),"[%name%: &[%artist% - ]%title%]|%name%|[%artist% - ]%title%|%shortfile%|");		
}


/* destory the preferences window */
void preferences_window_destroy()
{
	GtkWidget *dialog = glade_xml_get_widget(xml_preferences_window, "preferences_window");
	gtk_widget_destroy(dialog);
	g_object_unref(xml_preferences_window);
	xml_preferences_window = NULL;
	running = 0;
}

void update_preferences_information()
{
	cfg_set_single_value_as_string(config,"connection","hostname", 
			(char *)gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "hostname_entry"))));
	cfg_set_single_value_as_int(config, "connection", "portnumber",
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "port_spin"))));
	cfg_set_single_value_as_float(config,"connection","timeout",
		       	gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "timeout_spin"))));

}

void preferences_window_autoconnect(GtkToggleButton *tog)
{
	cfg_set_single_value_as_int(config, "connection", "autoconnect", gtk_toggle_button_get_active(tog));
}

void preferences_window_connect(GtkWidget *but)
{
	if(debug)g_print("**DEBUG** connect\n");
	if(info.connection == NULL)
		if(!connect_to_mpd())
		{
			info.conlock = FALSE;
			gtk_timeout_remove(update_timeout);
			update_timeout =  gtk_timeout_add(400, (GSourceFunc)update_interface, NULL);
			if(info.stats != NULL)
			{
				gchar *buffer = ctime(&info.stats->dbUpdateTime);
				/* nasty but I need to get rid of the trailing new line */
				buffer[strlen(buffer)-1]='\0';
				gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml_preferences_window, "db_lu")),buffer);
			}
		}

}

void preferences_window_disconnect(GtkWidget *but)
{
	if(debug)g_print("**DEBUG** disconnect\n");    
	disconnect_to_mpd();
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml_preferences_window, "db_lu")),"n/a");
}

/* this function is called from the main loop, it makes sure stuff is up-to-date(r) */
void preferences_update()
{
	if(!running)return;
	if((info.connection == NULL? 0:1) != connected)
	{
		if(info.connection == NULL)
		{
			gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "bt_con"), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "bt_dis"), FALSE);	    
			gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "vb_server_set"), FALSE);
			gtk_widget_show(glade_xml_get_widget(xml_preferences_window, "hb_warning_mesg"));
		}
		else
		{
			gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "bt_con"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "bt_dis"), TRUE);	   
			gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "vb_server_set"), TRUE);
			gtk_widget_hide(glade_xml_get_widget(xml_preferences_window, "hb_warning_mesg"));
		}
		update_outputs_settings();
		connected = (info.connection == NULL? 0:1);
	} 
	if(info.stats != NULL)
	{
		/* TODO: fix this to be nice */
//		if(last_db != info.stats->dbUpdateTime)
//		{
//			gchar *buffer = ctime(&info.stats->dbUpdateTime);
			/* nasty but I need to get rid of the trailing new line */
//			buffer[strlen(buffer)-1]='\0';
//			gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml_preferences_window, "db_lu")),buffer);
//		}
	}
}

void popup_enable_toggled(GtkToggleButton *but)
{
	cfg_set_single_value_as_int(config, "tray-icon", "do-popup", gtk_toggle_button_get_active(but));
}

void popup_position_changed(GtkOptionMenu *om)
{
	cfg_set_single_value_as_int(config, "tray-icon", "popup-location", gtk_option_menu_get_history(om));
}

void popup_timeout_changed()
{
	//info.popup.timeout = gtk_spin_button_get_value_as_int(
	cfg_set_single_value_as_int(config, "tray-icon", "popup-timeout", 
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "popup_timeout"))));
}

void update_popup_settings()
{
	gtk_toggle_button_set_active((GtkToggleButton *)
			glade_xml_get_widget(xml_preferences_window, "ck_popup_enable"),
			cfg_get_single_value_as_int_with_default(config, "tray-icon", "do-popup", 1));
	gtk_option_menu_set_history((GtkOptionMenu *)
			glade_xml_get_widget(xml_preferences_window, "om_popup_position"),
			cfg_get_single_value_as_int_with_default(config, "tray-icon", "popup-location", 0));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "popup_timeout")),
			cfg_get_single_value_as_int_with_default(config, "tray-icon", "popup-timeout", 2));

}


void update_server_settings()
{
	if(info.connection == NULL)
	{
		gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "vb_server_set"), FALSE);
		gtk_widget_show(glade_xml_get_widget(xml_preferences_window, "hb_warning_mesg"));
		return;
	}
	else 
	{
		gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "vb_server_set"), TRUE);
		gtk_widget_hide(glade_xml_get_widget(xml_preferences_window, "hb_warning_mesg"));
	}	

	if(mpd_ob_status_get_crossfade(connection) == 0)
	{
		gtk_toggle_button_set_active((GtkToggleButton *)
				glade_xml_get_widget(xml_preferences_window, "cb_fading"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "sb_fade_time"), FALSE);
	}
	else {
		gtk_toggle_button_set_active((GtkToggleButton *)
				glade_xml_get_widget(xml_preferences_window, "cb_fading"), TRUE);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "sb_fade_time")), mpd_ob_status_get_crossfade(connection));
		gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "sb_fade_time"), TRUE);

	}
	update_outputs_settings();
}

/* this sets all the settings in the notification area preferences correct */
void update_tray_settings()
{
	gtk_toggle_button_set_active((GtkToggleButton *)
			glade_xml_get_widget(xml_preferences_window, "ck_tray_enable"), 
			cfg_get_single_value_as_int_with_default(config, "tray-icon", "enable", 1));
}

void tray_enable_toggled(GtkToggleButton *but)
{
	g_print("changing tray icon %i\n", gtk_toggle_button_get_active(but));
	cfg_set_single_value_as_int(config, "tray-icon", "enable", (int)gtk_toggle_button_get_active(but));
	if(cfg_get_single_value_as_int_with_default(config, "tray-icon", "enable", 1))
	{
		create_tray_icon();
	}
	else
	{
		destroy_tray_icon();
	}
}

void entry_auth_changed(GtkEntry *entry)
{
	cfg_set_single_value_as_string(config, "connection","password",
		(char *)gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "entry_auth"))));
}

void auth_enable_toggled(GtkToggleButton *but)
{
	cfg_set_single_value_as_int(config, "connection", "useauth",gtk_toggle_button_get_active(but));
	gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "entry_auth"), 
			cfg_get_single_value_as_int_with_default(config, "connection","useauth",0));	
}

void xfade_enable_toggled(GtkToggleButton *but)
{

	int bool1  = gtk_toggle_button_get_active(but);
	gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "sb_fade_time"), bool1);
	if(bool1)
	{
		int fade_time = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "sb_fade_time")));	
		mpd_ob_status_set_crossfade(connection, fade_time);
	}
	else 
	{
		mpd_ob_status_set_crossfade(connection, 0);
	}	
}

void xfade_time_changed(GtkSpinButton *but)
{
	int fade_time = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget(xml_preferences_window, "sb_fade_time")));	
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(xml_preferences_window, "cb_fading"))))
	{
		return;
	}
	mpd_ob_status_set_crossfade(connection, fade_time);
}



/* this sets all the settings in the authentification area preferences correct */
void update_auth_settings()
{
	gtk_toggle_button_set_active((GtkToggleButton *)
			glade_xml_get_widget(xml_preferences_window, "ck_auth"), 
			cfg_get_single_value_as_int_with_default(config, "connection", "useauth", 0));
	gtk_widget_set_sensitive(glade_xml_get_widget(xml_preferences_window, "entry_auth"), 
			cfg_get_single_value_as_int_with_default(config, "connection", "useauth", 0));
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml_preferences_window, "entry_auth")),
			cfg_get_single_value_as_string_with_default(config, "connection","password", ""));
}

void osb_enable_tb(GtkToggleButton *but)
{

	int bool1  = gtk_toggle_button_get_active(but);
	cfg_set_single_value_as_int(config, "osb","enable", bool1);
}

void create_outputs_tree()
{
        GtkListStore *model;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *col;
	GtkTreeView *tree;

	tree = GTK_TREE_VIEW(glade_xml_get_widget(xml_preferences_window, "tv_outputs"));
	model = gtk_list_store_new(3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_INT);
	gtk_tree_view_set_model(tree, GTK_TREE_MODEL(model));
	g_object_unref(G_OBJECT(model));

	cell = gtk_cell_renderer_toggle_new();
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(col, cell, TRUE);
	gtk_tree_view_column_add_attribute(col, cell, "active", 0);
	gtk_tree_view_column_set_title(col, "Enabled");
	gtk_tree_view_append_column(tree, col);
	gtk_signal_connect(GTK_OBJECT(cell), "toggled",	GTK_SIGNAL_FUNC(outputs_toggled), tree);

	cell = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(col, cell, TRUE);
	gtk_tree_view_column_add_attribute(col, cell, "text", 1);
	gtk_tree_view_column_set_title(col, "Name");
	gtk_tree_view_append_column(tree, col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_visible(col, FALSE);
	gtk_tree_view_append_column(tree, col);
}

void outputs_toggled(GtkCellRendererToggle *cell, gchar *path_str, GtkTreeView *view)
{
        gboolean enabled;
	gint id;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(view);
	GtkTreePath *path = gtk_tree_path_new_from_string(path_str);

	if(gtk_tree_model_get_iter(model, &iter, path))
        {
	        gtk_tree_model_get(model, &iter, ENABLED_COL, &enabled, -1);
		gtk_tree_model_get(model, &iter, ID_COL, &id, -1);
		if(enabled)
                {
		        enabled = FALSE;
			mpd_sendDisableOutputCommand(connection->connection, id);
		}
		else
		{
		        enabled = TRUE;
			mpd_sendEnableOutputCommand(connection->connection, id);
		}
		mpd_finishCommand(connection->connection);          
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, ENABLED_COL, enabled, -1);
	}
	gtk_tree_path_free(path);
}

void update_outputs_settings()
{
        mpd_OutputEntity *output;
	GtkTreeIter iter;
	GtkListStore *store;
	GtkFrame *frame;

	frame = GTK_FRAME(glade_xml_get_widget(xml_preferences_window, "frm_outputs"));
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(xml_preferences_window, "tv_outputs"))));
	gtk_list_store_clear(store);
	if(info.connection && 
	   (info.connection->version[0] > 0 ||
	    info.connection->version[1] >= 12))
	{
   	        gtk_widget_show_all(GTK_WIDGET(frame));
	        mpd_sendOutputsCommand(info.connection);
		while((output = mpd_getNextOutput(info.connection)) != NULL)
		{
		        gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, 0, output->enabled?TRUE:FALSE, 1, output->name, 2, output->id, -1);
			mpd_freeOutputElement(output);
		}
	}
	else
	{
	        gtk_widget_hide_all(GTK_WIDGET(frame));
	}
}
