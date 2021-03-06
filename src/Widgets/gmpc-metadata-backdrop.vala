/* Gnome Music Player Client
 * Copyright (C) 2011-2012 Qball Cow <qball@gmpclient.org>
 * Project homepage: http://gmpclient.org/

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
using GLib;
using Gmpc;

private const string log_domain_mdbd = "Gmpc.Widgets.MetaData.Backdrop";
namespace Gmpc
{
    namespace MetaData.Widgets
    {
        class Backdrop : Gtk.EventBox
        {
            private string song_checksum = null;
            private MPD.Song? cur_song = null;
            private Gmpc.MetaData.Type cur_type = Gmpc.MetaData.Type.BACKDROP_ART;
            /*   */
            private Gdk.Pixbuf pb = null;
            private ModificationType mod_type = (ModificationType)
                                                config.get_int_with_default(
                                                    "Backdrop", "alterations",2);

            // Image loading
            private Gmpc.PixbufLoaderAsync? loader = null;
            private void set_from_item (Gmpc.MetaData.Item? item)
            {
                pb = null;
                if(item == null || song_checksum == null)
                {
                    this.queue_draw();
                    return;
                }
                if (item.content_type == Gmpc.MetaData.ContentType.RAW)
                {
                    Gtk.Allocation req;
                    int width;
                    this.get_allocation(out req);
                    width = int.max(req.width, 400);
                    log(log_domain_mdbd, GLib.LogLevelFlags.LEVEL_DEBUG,
                        "Getting image with size: %u", width);
                    if(loader == null)
                        loader = new PixbufLoaderAsync();
                    loader.pixbuf_update.connect((source, pixbuf)=>
                    {
                        this.pb = pixbuf;
                        log (log_domain_mdbd,GLib.LogLevelFlags.LEVEL_DEBUG,
                        "Updating background");
                        this.queue_draw();
                    });
                    unowned uchar[] data = item.get_raw();
                    loader.set_from_raw(data, width,-1, mod_type, item.md5sum);
                }
            }
            /**
             * @param The #MPD.Song to set the background for or NULL to clear.
             *
             * Set the background for song, or clear the background.
             */
            public void set_song(MPD.Song? song)
            {
                cur_song = song.copy();
                if(song == null)
                {
                    song_checksum = null;
                    set_from_item(null);
                    return;
                }
                // Same artist/bg do not update.
                if(song_checksum == Gmpc.Misc.song_checksum_type(song, cur_type))
                    return;

                song_checksum = Gmpc.Misc.song_checksum_type(song, cur_type);
                Gmpc.MetaData.Item item = null;
                var a = Gmpc.MetaData.get_path(song, cur_type,out item); //metawatcher.query(song, cur_type, out item);
                if(a == Gmpc.MetaData.Result.AVAILABLE)
                {
                    this.set_from_item(item);
                }
                else
                {
                    this.set_from_item(null);
                }
            }
            /**
             * @param type The type of backdrop (artist or album image)
             * Create backdrop widget of type.
             * @return a new #Backdrop (of type #Gtk.Eventbox)
             */
            public Backdrop(Gmpc.MetaData.Type type)
            {
                assert(type == Gmpc.MetaData.Type.BACKDROP_ART ||
                       type == Gmpc.MetaData.Type.ARTIST_ART||
                       type == Gmpc.MetaData.Type.ALBUM_ART);
                cur_type = type;

/*
                this.realize.connect((source)=>
                {
                    source.get_window().set_back_pixmap(null, true);
                });
*/
                // Set visible window
                this.set_visible_window(true);
                // Set paintable
                this.set_app_paintable(true);
                // Watch changes */
                metawatcher.data_changed.connect((csong, type, result, met) =>
                {
                    if(song_checksum != null &&
                    type == cur_type &&
                    song_checksum == Gmpc.Misc.song_checksum_type(csong,cur_type))
                    {
                        if(met != null && result == Gmpc.MetaData.Result.AVAILABLE)
                        {
                            this.set_from_item(met);
                        }
                        else
                        {
                            this.set_from_item(null);
                        }
                    }
                });

                // Add expose event
                this.draw.connect(container_expose);

                this.button_press_event.connect(button_press_event_callback);
            }
            private bool button_press_event_callback(Gdk.EventButton event)
            {
                if(event.button != 3) return false;
                if(cur_song == null) return false;
                if(cur_song.artist == null) return false;
                var menu = new Gtk.Menu();

                var item =  new Gtk.ImageMenuItem.with_label(_("Refresh backdrop"));
                item.set_image(new Gtk.Image.from_stock("gtk-refresh", Gtk.IconSize.MENU));
                item.activate.connect((source)=>
                {
                    MetaData.Item mitem = null;
                    stdout.printf("Push backdrop update\n");
                    var a = MetaData.get_path(cur_song, cur_type|Gmpc.MetaData.Type.QUERY_NO_CACHE, out mitem);
                    //metawatcher.query(cur_song, cur_type|Gmpc.MetaData.Type.QUERY_NO_CACHE, out mitem);
                    if(a == Gmpc.MetaData.Result.AVAILABLE)
                    {
                        this.set_from_item(mitem);
                    }
                    else {
                        this.set_from_item(null);
                    }
                });
                menu.append(item);
                /*  Add selector */
                item = new Gtk.ImageMenuItem.with_label(_("Metadata selector"));
                item.set_image(new Gtk.Image.from_stock("gtk-edit", Gtk.IconSize.MENU));
                item.activate.connect((source)=>
                {
                    new Gmpc.MetaData.EditWindow(cur_song, cur_type);
                });
                menu.append(item);
                menu.show_all();
                menu.popup(null, null, null, event.button, event.time);
                return true;
            }
            /**
             * Draw the background. (only exposed part)
             */
            private bool container_expose(Gtk.Widget ev, Cairo.Context gc) 
            {
                Gtk.Allocation alloc;
                ev.get_allocation(out alloc);
                gc.rectangle(alloc.x, alloc.y, alloc.width, alloc.height);
                /* If there is an background image set. */
                if(pb != null)
                {
                    gc.set_source_rgb(0.0,0.0,0.0);
                    gc.fill_preserve();
                    Gdk.cairo_set_source_pixbuf(gc, pb,0.0,0.0);
                    gc.fill();
                }
                /* no image set */
                else
                {
                    gc.set_source_rgb(0.0,0.0,0.0);
                    gc.fill();
                }
                return false;
            }
        }
    }
}
