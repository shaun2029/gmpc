
namespace Gmpc {
    [CCode (cname="TRUE",cheader_filename="gtk/gtk.h,gtktransition.h")]
    static const bool use_transition;

    [CCode (cname="browsers_metadata")]
    static Gmpc.Browsers.Metadata browsers_metadata;

    [CCode (cname="tag2_browser_update_orientation")]
    static void tag2_browser_update_orientation();

    [CCode (cname="paned_size_group", cheader_filename="plugin.h")]
    static PanedSizeGroup paned_size_group;

    [CCode (cname = "gmpcconn", cheader_filename="main.h")]
    static Connection gmpcconn;

    [CCode (cname = "connection", cheader_filename="plugin.h")]
    static MPD.Server server;

    [CCode (cname = "gmw", cheader_filename="main.h")]
    static Meta.Watcher metawatcher;

    [CCode (cname = "gmpc_profiles", cheader_filename="plugin.h")]
    static Profiles profiles;

    [CCode (cname = "gmpc_easy_command", cheader_filename="plugin.h")]
    static Easy.Command  easy_command;

	[CCode (cname = "playlist", cheader_filename="main.h")]
	static Gmpc.MpdData.ModelPlaylist playlist;


   [CCode (cheader_filename="metadata.h")]
   namespace MetaData {
        [CCode (cname="MetaDataContentType", cprefix = "META_DATA_CONTENT_", cheader_filename = "libmpd/libmpd.h,metadata.h")]
        public enum ContentType {
            EMPTY,
            URI,
            TEXT,
            RAW,
            HTML,
            STRV,
            TEXT_LIST
        }

        [CCode (cname="MetaData",free_function="meta_data_free",copy_function="meta_data_dup",has_type_id = false)]
        [Compact]
        public class Item {
            [CCode (cname="meta_data_new")]
            public Item ();
            public Gmpc.MetaData.Type type;
            [CCode (cname="meta_data_dup")]
            public static Item copy(Item item);
			[CCode (cname="meta_data_dup")]
			public Item dup();


           public unowned string plugin_name;
           public int size;
           public void * content;
		   [CCode (array_length = false)]
		   public uchar[] md5sum;
		   public Gmpc.MetaData.ContentType content_type;

           [CCode (cname="meta_data_is_empty")]
           public bool is_empty();


           /* add accessors? */
           [CCode (cname="meta_data_get_raw")]
           public unowned uchar[] get_raw();
	   [CCode (cname="meta_data_set_raw")]
	   public void set_raw(uchar[] data);
	   [CCode (cname="meta_data_set_raw_owned")]
	   public void set_raw_void(ref void *data,ref uint length);

           [CCode (cname="meta_data_get_text")]
           public unowned string  get_text();
           [CCode (cname="meta_data_set_text")]
           public void set_text(string data);

           [CCode (cname="meta_data_get_text_from_html")]
           public string get_text_from_html();
           /* same as get_text */

           [CCode (cname="meta_data_get_uri")]
           public unowned string? get_uri();
           [CCode (cname="meta_data_set_uri")]
           public void set_uri(string uri);
           [CCode (cname="meta_data_get_thumbnail_uri")]
           public unowned string? get_thumbnail_uri();
           [CCode (cname="meta_data_set_thumbnail_uri")]
           public void set_thumbnail_uri(string uri);

           [CCode (cname="meta_data_get_html")]
           public unowned string get_html();

           [CCode (cname="meta_data_get_text_vector")]
           public unowned string[] get_text_vector();

           [CCode (cname="meta_data_get_text_list")]
           public unowned GLib.List<unowned string> get_text_list();

           [CCode (cname="meta_data_is_text_list")]
           public bool  is_text_list();
           [CCode (cname="meta_data_dup_steal")]
           public MetaData.Item dup_steal();

        }

        [CCode (cname="MetaDataType", cprefix = "META_", cheader_filename = "metadata.h", has_type_id = false)]
        public enum Type {
            ALBUM_ART       = 1,
            ARTIST_ART      = 2,
            ALBUM_TXT       = 4,
            ARTIST_TXT      = 8,
            SONG_TXT        = 16,
            ARTIST_SIMILAR  = 32,
            SONG_SIMILAR    = 64,
            GENRE_SIMILAR   = 128,
            SONG_GUITAR_TAB = 256,
			BACKDROP_ART	= 512,
            QUERY_DATA_TYPES = 65535,
            QUERY_NO_CACHE   = 65536
        }

        [CCode (cname="MetaDataResult", cprefix = "META_DATA_", cheader_filename = "metadata.h", has_type_id = false)]
        public enum Result {
            AVAILABLE,
            UNAVAILABLE,
            FETCHING
        }

        [CCode (cname="meta_data_get_path")]
        public MetaData.Result get_path(MPD.Song song, Type type, out MetaData.Item item, Callback? callback=null);

        public delegate void Callback (void *handle,string? plugin_name, GLib.List<MetaData.Item>? list);
        [CCode ( cname="metadata_get_list", cheader_filename="libmpd/libmpd.h,metadata.h" )]
        public void* get_list(MPD.Song song, Type type, Callback callback);

        [CCode ( cname="metadata_get_list_cancel", cheader_filename="metadata.h" )]
        public void* get_list_cancel(void *handle);


        [CCode ( cname="meta_data_set_entry", cheader_filename="metadata.h")]
        public void set_metadata(MPD.Song song, Gmpc.MetaData.Item met);

        [CCode ( cname="gmpc_get_metadata_filename", cheader_filename="libmpd/libmpd.h,metadata.h")]
        public string get_metadata_filename(Type type, MPD.Song song, string? extension);

        [CCode ( cname="GmpcMetaImage", cheader_filename="gmpc-metaimage.h")]
        public class Image: Gtk.Widget {
			public int size;
            [CCode (cname="gmpc_metaimage_new_size")]
            public Image(Type type, int size);
            [CCode (cname="gmpc_metaimage_update_cover_from_song")]
            public void update_from_song(MPD.Song song);

            [CCode (cname="gmpc_metaimage_update_cover_from_song_delayed")]
            public void update_from_song_delayed(MPD.Song song);
            [CCode (cname="gmpc_metaimage_set_squared")]
            public void set_squared(bool squared);
            [CCode (cname="gmpc_metaimage_set_hide_on_na")]
            public void set_hide_on_na(bool hide);
            [CCode (cname="gmpc_metaimage_set_no_cover_icon")]
            public void set_no_cover_icon(string name);
            [CCode (cname="gmpc_metaimage_set_loading_cover_icon")]
            public void set_loading_cover_icon(string name);

            [CCode (cname="gmpc_metaimage_set_cover_na")]
            public void set_cover_na();
            [CCode (cname="gmpc_metaimage_set_scale_up")]
            public void set_scale_up(bool scale);

            [CCode (cname="gmpc_metaimage_set_dirty")]
            public void set_dirty();
        }

        [CCode ( cname="GmpcMetaTextView", cheader_filename="gmpc-meta-text-view.h")]
        public class TextView: Gtk.TextView {
                public bool use_monospace;
            [CCode (cname="gmpc_meta_text_view_new")]
                public TextView(Type type);
            [CCode (cname="gmpc_meta_text_view_query_text_from_song")]
                public void query_from_song(MPD.Song song);
                public bool force_ro;
        }

   }

   namespace Messages {
       [CCode (cprefix = "ERROR_", cheader_filename = "playlist3-messages.h")]
       public enum Level{
        INFO,
        WARNING,
        CRITICAL
    }

    [CCode (cname = "playlist3_show_error_message", cheader_filename="playlist3-messages.h")]
    void show(string message, Gmpc.Messages.Level level);

    [CCode (cname = "playlist3_error_add_widget", cheader_filename="playlist3-messages.h")]
	void add_widget(Gtk.Widget widget);
   }

   namespace AsyncDownload {
     [CCode (cname="GEADStatus",cprefix = "GEAD_", cheader_filename = "gmpc_easy_download.h")]
        public enum Status {
            DONE,
            PROGRESS,
            FAILED,
            CANCELLED
        }

        [CCode (cname="GEADAsyncHandler", cheader_filename="gmpc_easy_download.h",ref_function="", unref_function ="")]
        [Compact]
        [Immutable]
        public class Handle {
            [CCode (cname="gmpc_easy_async_cancel", cheader_filename="gmpc_easy_download.h")]
            public void cancel ();
            /* Gets raw data. Remember data_length does not include trailing \0. */
            [CCode (cname="gmpc_easy_handler_get_data_vala_wrap", cheader_filename="gmpc_easy_download.h")]
            public unowned uchar[] get_data();

            [CCode (cname="gmpc_easy_handler_get_data_as_string", cheader_filename="gmpc_easy_download.h")]
            public unowned string get_data_as_string();

            [CCode (cname="gmpc_easy_handler_get_uri", cheader_filename="gmpc_easy_download.h")]
            public unowned string get_uri();

            [CCode (cname="gmpc_easy_handler_get_user_data", cheader_filename="gmpc_easy_download.h")]
            public void * get_user_data();

            [CCode (cname="gmpc_easy_handler_set_user_data", cheader_filename="gmpc_easy_download.h")]
            public void set_user_data(void *user_data);

            [CCode (cname="gmpc_easy_handler_get_content_size", cheader_filename="gmpc_easy_download.h")]
            public int64 get_content_size();
        }


        public delegate void Callback (Gmpc.AsyncDownload.Handle handle, Gmpc.AsyncDownload.Status status);
        public delegate void CallbackVala (Gmpc.AsyncDownload.Handle handle, Gmpc.AsyncDownload.Status status, void *p);

        [CCode (cname="gmpc_easy_async_downloader_vala", cheader_filename="gmpc_easy_download.h")]
        public unowned Gmpc.AsyncDownload.Handle download_vala(string uri, void *p,Gmpc.AsyncDownload.CallbackVala callback);
        [CCode (cname="gmpc_easy_async_downloader", cheader_filename="gmpc_easy_download.h")]
        public unowned Gmpc.AsyncDownload.Handle download(string uri, Gmpc.AsyncDownload.Callback callback);

        [CCode (cname="gmpc_easy_download_uri_escape", cheader_filename="gmpc_easy_download.h")]
        public string escape_uri(string part);
   }

    [CCode (cname="gmpc_get_full_glade_path", cheader_filename="plugin.h")]
    public string data_path(string file);
    [CCode (cname="gmpc_get_user_path", cheader_filename="plugin.h")]
    public string user_path(string file);
    [CCode (cname="open_uri", cheader_filename="misc.h")]
    public void open_uri(string uri);
    [CCode (cname="open_help", cheader_filename="misc.h")]
	public void open_help(string uri);

    namespace Playlist {
		[CCode (cname="Pl3CatBrowserType", cprefix="PL3_CAT_BROWSER_", cheader_filename="plugin.h")]
		public enum BrowserType{
			TOP,
			LIBRARY,
			ONLINE_MEDIA,
			MISC
		}
        [CCode (cname="(GtkWindow *)playlist3_get_window", cheader_filename="plugin.h")]
        public unowned Gtk.Window get_window();
        [CCode (cname="playlist3_window_is_hidden", cheader_filename="plugin.h")]
        public bool is_hidden();

        [CCode (cname="pl3_hide", cheader_filename="plugin.h")]
        public void hide();

        [CCode (cname="create_playlist3", cheader_filename="plugin.h")]
        public void show();
        [CCode (cname="playlist3_get_accel_group", cheader_filename="playlist3.h")]
        public unowned Gtk.AccelGroup get_accel_group();

        [CCode (cname="playlist3_get_widget_by_id", cheader_filename="playlist3.h")]
        public Gtk.Widget get_widget_by_id(string id);

		[CCode (cname="playlist3_get_sidebar_state", cheader_filename="plugin.h")]
		public Gmpc.Plugin.SidebarState get_sidebar_state();
    }

    namespace TrayIcon2 {
        [CCode (cname="trayicon2_toggle_use_appindicator", cheader_filename="tray-icon2.h")]
        public void toggle_use_appindicator();

        [CCode (cname="trayicon2_have_appindicator_support", cheader_filename="tray-icon2.h")]
        public bool have_appindicator_support();
    }

	[CCode (cname = "config", cheader_filename="plugin.h")]
	static Settings config;
    [CCode (cheader_filename="config1.h",cname="config_obj", free_function="cfg_close")]
        [Compact]
        [Immutable]
    public class Settings {
        [CCode (cname="cfg_open", cheader_filename="config1.h")]
        public Settings.from_file(string file);
        [CCode (cname="cfg_get_single_value_as_string_with_default", cheader_filename="config1.h")]
        public string get_string_with_default(string class, string key, string value);
        [CCode (cname="cfg_get_single_value_as_string", cheader_filename="config1.h")]
        public string? get_string(string @class, string key);
        [CCode (cname="cfg_get_single_value_as_int_with_default", cheader_filename="config1.h")]
        public int get_int_with_default(string class, string key, int value);
        [CCode (cname="cfg_get_single_value_as_int", cheader_filename="config1.h")]
        public int get_int(string @class, string key);
        [CCode (cname="cfg_get_single_value_as_int", cheader_filename="config1.h")]
        public bool get_bool(string @class, string key);
        [CCode (cname="cfg_get_single_value_as_int_with_default", cheader_filename="config1.h")]
        public bool get_bool_with_default(string @class, string key, bool value);
        [CCode (cname="cfg_set_single_value_as_string", cheader_filename="config1.h")]
        public void set_string(string @class, string key, string @value);
        [CCode (cname="cfg_set_single_value_as_int", cheader_filename="config1.h")]
        public int set_int(string class, string key, int value);
        [CCode (cname="cfg_set_single_value_as_int", cheader_filename="config1.h")]
        public int set_bool(string @class, string key, bool @value);
        [CCode (cname="cfg_get_class_list", cheader_filename="config1.h")]
        public SettingsList get_class_list();
        [CCode (cname="cfg_remove_class", cheader_filename="config1.h")]
        public void remove_class(string key);
    }

    [CCode (cheader_filename="config1.h", cname="conf_mult_obj", free_function="cfg_free_multiple")]
        [Compact]
        [Immutable]
    public class SettingsList {
        public string key;
        public string @value;
        public SettingsList next;
        public unowned SettingsList prev;
    }

    namespace Misc{
        [CCode (cname="colorshift_pixbuf",cheader_filename="misc.h")]
        public void colorshift_pixbuf(Gdk.Pixbuf dest, Gdk.Pixbuf src, int shift);

        [CCode (cname="darken_pixbuf",cheader_filename="misc.h")]
        public void darken_pixbuf(Gdk.Pixbuf dest, uint factor = 1);
        [CCode (cname="decolor_pixbuf",cheader_filename="misc.h")]
        public void decolor_pixbuf(Gdk.Pixbuf dest, Gdk.Pixbuf src);
		[CCode (cname="screenshot_add_border",cheader_filename="misc.h")]
		public void border_pixbuf (Gdk.Pixbuf buf);
        [CCode (cname="misc_header_expose_event",cheader_filename="misc.h")]
        public bool misc_header_expose_event(Gtk.Widget widget, Cairo.Context ct);


        [CCode (cname="format_time_real", cheader_filename="misc.h")]
        public string format_time(ulong seconds, string pre="");
        [CCode (cname="format_time_real_newline", cheader_filename="misc.h")]
        public string format_time_newline(ulong seconds, string pre="");


        [CCode (cname="mpd_song_checksum",cheader_filename="misc.h")]
        public string? song_checksum(MPD.Song? song);
        [CCode (cname="mpd_song_checksum_type",cheader_filename="misc.h")]
        public string? song_checksum_type(MPD.Song? song, Gmpc.MetaData.Type type);
    }

    /* Browser */
    namespace Browser{
        [CCode (cname="playlist3_insert_browser")]
        public void insert(out Gtk.TreeIter iter, int position);

        namespace File {
            [CCode (cname="pl3_file_browser_open_path")]
            public void open_path(string path);
        }

        namespace Find {
            [CCode (cname="pl3_find2_ec_database")]
            public void query_database(void *user_data, string query);
        }

        namespace Metadata {
            [CCode (cname="info2_activate")]
            public void show();
            [CCode (cname="info2_fill_song_view")]
            public void show_song(MPD.Song song);
            [CCode (cname="info2_fill_artist_view")]
            public void show_artist(string artist);

            [CCode (cname="info2_fill_album_view")]
            public void show_album(string artist,string album);
        }
        namespace PlaylistEditor {
        

            public delegate void Callback (Gtk.MenuItem item);
            [CCode (cname="playlist_editor_right_mouse")]
            public void right_mouse(Gtk.Menu menu, Callback cb);
        }
    }

    namespace Playlist3 {
        [CCode (cname="playlist3_get_category_tree_view")]
        public unowned Gtk.TreeView get_category_tree_view();

        [CCode (cname="main_window_add_status_icon")]
        public void add_status_icon(Gtk.Widget widget);

        [CCode (cname="pl3_update_go_menu",cheader_filename="plugin.h")]
        public void update_go_menu();
    }

    /* objects */
    namespace MpdData {

        [CCode (cheader_filename="gmpc-mpddata-model.h")]
        public class Model : GLib.Object, Gtk.TreeModel{
            [CCode (has_construct_function = true,cname="gmpc_mpddata_model_new")]
            public Model();

            [CCode (cname="gmpc_mpddata_model_set_mpd_data")]
            public void set_mpd_data(owned MPD.Data.Item? list);

            [CCode (cname="gmpc_mpddata_model_set_request_artist")]
            public void set_request_artist(string? list);

            [CCode (cname="gmpc_mpddata_model_get_request_artist")]
            public unowned string get_request_artist();
            public int icon_size;
        }

        [CCode (cheader_filename="gmpc-mpddata-model-playlist.h",type_check_function="GMPC_IS_MPDDATA_MODEL_PLAYLIST", type_id="GMPC_TYPE_MPDDATA_MODEL_PLAYLIST")]
        public class ModelPlaylist : Gmpc.MpdData.Model {
			public signal void total_playtime_changed(ulong loaded_songs, ulong total_playtime);

            [CCode (cname="gmpc_mpddata_model_playlist_get_total_playtime")]
			public void get_total_playtime( out ulong loaded_song, out ulong total_playtime);
    
            [CCode (cname="gmpc_mpddata_model_playlist_is_current_song")]
            public bool is_current_song(Gtk.TreeIter iter);
		}

        [CCode (cname="", cprefix = "MPDDATA_MODEL_", cheader_filename = "gmpc-mpddata-model.h")]
            public enum ColumnTypes {
                    COL_MPDSONG,
                    COL_PATH,
                    COL_MARKUP,
                    COL_SONG_ARTIST,
                    COL_SONG_ALBUM,			      /* album name */
                    COL_SONG_TITLE,			      /* song title */
                    COL_SONG_TITLEFILE,		    /* song title */
                    COL_SONG_GENRE,			      /* song genre */
                    COL_SONG_TRACK,			      /* song track */
                    COL_SONG_NAME,			      /* stream name */
                    COL_SONG_COMPOSER,		    /* composer name */
                    COL_SONG_PERFORMER,		    /* performer */
                    COL_SONG_DATE,			      /* date */
                    COL_SONG_LENGTH_FORMAT,	  /* length formatted */
                    COL_SONG_DISC,			      /* disc */
                    COL_SONG_COMMENT,			    /* comment */
                    COL_ICON_ID,				      /* icon id */
                    COL_SONG_ID,
                    COL_SONG_POS,
                    COL_SONG_ALBUMARTIST,
                    COL_PATH_EXTENSION,				/* Extension */
                    COL_PATH_DIRECTORY,				/* Directory */
                    COL_SONG_PRIORITY,
                    COL_SONG_RATING,
                    ROW_TYPE,
            }

    }


    namespace MpdInteraction {
        [CCode (cname="play_path",cheader_filename="mpdinteraction.h")]
        public void play_path(string path);
        [CCode (cname="submenu_for_song")]
        public void submenu_for_song(Gtk.Widget menu, MPD.Song song);

        [CCode (cname="connection_set_current_profile")]
        public void set_current_profile(string id);

		[CCode (cname="connect_to_mpd")]
		public void connect();
        [CCode (cname="disconnect_from_mpd")]
        public void disconnect();
    }

    namespace Fix{
        [CCode (cname="gdk_pixbuf_loader_write", cheader_filename="gdk-pixbuf/gdk-pixdata.h")]
            public void write_loader(Gdk.PixbufLoader loader, string data, size_t size) throws GLib.Error;

        [CCode (cname="screenshot_add_border", cheader_filename="misc.h")]
            public void add_border(Gdk.Pixbuf image);

        [CCode (cname="pango_attr_list_change", cheader_filename="pango/pango.h")]
            public void change (Pango.AttrList list,owned Pango.Attribute attr);
    }

    [CCode (cheader_filename="pixbuf-cache.h")]
    namespace PixbufCache {
        [CCode (cname="pixbuf_cache_lookup_icon")]
            public Gdk.Pixbuf? lookup_icon(int size,[CCode (array_length = false)] uchar[] url);
        [CCode (cname="pixbuf_cache_add_icon")]
            public void add_icon(int size,[CCode (array_length = false)] uchar[] url, Gdk.Pixbuf pb);

    }

    [CCode (cheader_filename="advanced-search.h")]
    namespace Query{
        [CCode (cname="advanced_search")]
        public MPD.Data.Item? search(string query, bool playlist);

    }

	[CCode (cname="gmpcPluginParent",cprefix="gmpc_plugin_",cheader_filename="plugin-internal.h")]
        [Compact]
        [Immutable]
	public class ParentPlugin
	{
		public int get_id();
		public unowned string get_name();
		public bool get_enabled();
		public bool has_enabled();
		public void set_enabled(int e);
		public bool is_browser();
        public int tool_menu_integration(Gtk.Menu menu);
        public int browser_cat_right_mouse_menu(Gtk.Menu menu, int type, Gtk.TreeView tree, Gdk.EventButton button);
        public int browser_song_list_option_menu(Gtk.TreeView tree, Gtk.Menu menu);
	}
	[CCode (cheader_filename="main.h", array_length = false, cname="plugins")]
	static weak ParentPlugin[] plugins;
	[CCode (cname="num_plugins",cheader_filename="main.h")]
	static int num_plugins;

    namespace Preferences {
        [CCode (cname="preferences_window_update", cheader_filename="preferences.h")]
        public void update();
		[CCode (cname="preferences_show_pref_window", cheader_filename="preferences.h")]
		public void show(int id);
    }
}
