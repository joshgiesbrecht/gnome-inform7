/*  Copyright 2006 P.F. Chimento
 *  This file is part of GNOME Inform 7.
 * 
 *  GNOME Inform 7 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GNOME Inform 7 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNOME Inform 7; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <gnome.h>
#include <string.h>

#include "extension.h"
#include "interface.h"
#include "tabsource.h"
#include "support.h"
#include "windowlist.h"
#include "prefs.h"

static GSList *extlist = NULL;

/* Create a new representation of an extension file and add it to the list */
struct extension *new_ext() {
    struct extension *newext = g_malloc(sizeof(struct extension));

    newext->filename = NULL;
    newext->window = create_ext_window();
    newext->buffer = create_natural_inform_source_buffer();

    /* Connect the source buffer to our sourceview */
    gtk_text_view_set_buffer(
      GTK_TEXT_VIEW(lookup_widget(newext->window, "ext_code")),
      GTK_TEXT_BUFFER(newext->buffer));

    extlist = g_slist_append(extlist, (gpointer)newext);
    update_window_list();
    
    return newext;
}

/* Remove an extension from the list and free it */
void delete_ext(struct extension *oldext) {
    extlist = g_slist_remove(extlist, (gconstpointer)oldext);

    if(oldext->filename != NULL)
        free(oldext->filename);
    gtk_widget_destroy(oldext->window);
    g_free(oldext);
    
    update_window_list();
}

/* Get the extension whose topwindow is the ancestor of widget */
struct extension *get_ext(GtkWidget *widget) {
    GSList *iter = extlist;
    /* the following is because the menu items do not share the same toplevel
     as the other widgets? */
    GtkWidget *topwindow = lookup_widget(widget, "ext_window");

    while(iter != NULL) {
        if(((struct extension *)(iter->data))->window == topwindow)
            return (struct extension *)(iter->data);
        iter = g_slist_next(iter);
    }
    return NULL;
}

/* Set the filename for this representation */
void set_ext_filename(struct extension *ext, gchar *filename) {
    if(ext->filename)
        g_free(ext->filename);
    ext->filename = g_strdup(filename);
    gtk_window_set_title(GTK_WINDOW(ext->window), strrchr(filename, '/') + 1);
    update_window_list();
}

/* Carry out func on each topwindow */
void for_each_extension_window(void (*func)(GtkWidget *)) {
    GSList *iter;
    for(iter = extlist; iter != NULL; iter = g_slist_next(iter))
        func(((struct extension *)(iter->data))->window);
}

/* Carry out func on each text buffer */
void for_each_extension_buffer(void (*func)(GtkSourceBuffer *)) {
    GSList *iter;
    for(iter = extlist; iter != NULL; iter = g_slist_next(iter))
        func(((struct extension *)(iter->data))->buffer);
}