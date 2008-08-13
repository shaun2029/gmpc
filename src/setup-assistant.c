#include <gtk/gtk.h>
#include "main.h"
#include "setup-assistant.h"

GtkWidget *con_pref = NULL;
gulong  connect_signal = 0;

static void destroy_assistant(GtkAssistant *assistant)
{
    /**
     * Destroy
     */

    if(con_pref)
    {
        connection_plug.pref->destroy(con_pref);
    }
    gtk_widget_destroy(GTK_WIDGET(assistant));
    if(connect_signal)
    {
        g_signal_handler_disconnect(G_OBJECT(gmpcconn), connect_signal);
        connect_signal = 0;
    }
}


static void close_assistant(GtkAssistant *assistant)
{
    /* Read out the configuration */


    destroy_assistant(assistant);
}

static void cancel_assistant(GtkAssistant *assistant)
{

    destroy_assistant(assistant);
}

static void connection_changed_assistant(GmpcConnection *gc,MpdObj *mi, int connect, gpointer data)
{
    GtkAssistant *assistant = GTK_ASSISTANT(data);
    if(con_pref)
    {
        gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), con_pref, connect);

    }
}


void setup_assistant(void)
{
    GtkWidget *page = NULL;
    GdkPixbuf *header_pb;
    GtkWidget *assistant = gtk_assistant_new();

    gtk_window_set_default_size (GTK_WINDOW (assistant), 400, 300);

    /**
     * Header image
     */
    header_pb = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), "gmpc", 48, 0,NULL);

    /**
     * Append an introduction page
     */
    page = gtk_label_new(_("It is the first time you have launched gmpc.\nThis assistant will help you connect gmpc to your mpd daemon."));
    gtk_assistant_append_page(GTK_ASSISTANT(assistant), page);
    gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), page, _("Welcome to GMPC"));
    gtk_assistant_set_page_header_image(GTK_ASSISTANT(assistant), page, header_pb);
    gtk_widget_show_all(page);
    gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), page, TRUE);

    /**
     * Add a configure page
     */
    page = con_pref = gtk_event_box_new(); 
    gtk_container_set_border_width(GTK_CONTAINER(page), 8);
    connection_plug.pref->construct(con_pref);
    gtk_widget_show (page);
    gtk_assistant_append_page (GTK_ASSISTANT (assistant), page);
    /* set up header */
    gtk_assistant_set_page_header_image(GTK_ASSISTANT(assistant), page, header_pb);
    gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), page, _("Setup connection"));
    /* setup page type */
    gtk_assistant_set_page_type  (GTK_ASSISTANT (assistant), page, GTK_ASSISTANT_PAGE_CONTENT);
    gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), page, FALSE);


    /**
     * End 
     */
    page = gtk_label_new(_("Gmpc is now ready for use.\nEnjoy using gmpc."));
    gtk_assistant_append_page(GTK_ASSISTANT(assistant), page);
    gtk_assistant_set_page_title(GTK_ASSISTANT(assistant), page, _("Finish"));
    gtk_assistant_set_page_header_image(GTK_ASSISTANT(assistant), page, header_pb);
    gtk_widget_show_all(page);
    gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), page, GTK_ASSISTANT_PAGE_SUMMARY);
    gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), page, TRUE);


    /* remove refernce to pixbuf */
    g_object_unref(header_pb);

    /**
     * Process it
     */

    gtk_window_set_transient_for(GTK_WINDOW(assistant),
            GTK_WINDOW(glade_xml_get_widget(pl3_xml, "pl3_win")));
    gtk_window_set_modal(GTK_WINDOW(assistant),TRUE);
    gtk_window_set_position(GTK_WINDOW(assistant),  GTK_WIN_POS_CENTER_ON_PARENT); 
    gtk_widget_show_all(GTK_WIDGET(assistant));

    g_signal_connect(G_OBJECT(assistant), "close", G_CALLBACK(close_assistant), NULL);
    g_signal_connect(G_OBJECT(assistant), "cancel", G_CALLBACK(cancel_assistant), NULL);



    connect_signal = g_signal_connect(G_OBJECT(gmpcconn), "connection_changed", G_CALLBACK(connection_changed_assistant),assistant);
}
