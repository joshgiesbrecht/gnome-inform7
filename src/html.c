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
#include <gtkhtml/gtkhtml.h>
#include <gtkhtml/gtkhtml-stream.h>

#include "html.h"
#include "appwindow.h"
#include "tabsource.h"
#include "story.h"
#include "support.h"
#include "prefs.h"
#include "error.h"

/* GtkHTML ignores and erases <script> tags; so, when we load an URL we keep the
source and store it in this table, so we can refer back to the original
source. Stupid and ugly, but necessary. */
static GSList *source_table = NULL;

/* Add the source of a HTML page to the table, with the GtkHTML widget as key*/
void source_table_add(GtkHTML *html, gchar *source) {
    g_return_if_fail(html != NULL);
    g_return_if_fail(source != NULL);
    
    struct source_entry *entry = g_malloc(sizeof(struct source_entry));
    entry->html = html;
    entry->source = g_strdup(source);
    source_table = g_slist_prepend(source_table, (gpointer)entry);
}

/* Remove an entry from the HTML source table */
void source_table_remove(GtkHTML *html) {
    g_return_if_fail(html != NULL);
    
    GSList *iter;
    for(iter = source_table; iter != NULL; iter = g_slist_next(iter))
        if(((struct source_entry *)(iter->data))->html == html) {
            g_free(((struct source_entry *)(iter->data))->source);
            source_table = g_slist_remove(source_table,
              (gconstpointer)(iter->data));
        }
}

/* The GtkHTML widget is now displaying another HTML file, change it in the
table */
void source_table_update(GtkHTML *html, gchar *source) {
    g_return_if_fail(html != NULL);
    g_return_if_fail(source != NULL);
    
    GSList *iter;
    for(iter = source_table; iter != NULL; iter = g_slist_next(iter))
        if(((struct source_entry *)(iter->data))->html == html) {
            g_free(((struct source_entry *)(iter->data))->source);
            ((struct source_entry *)(iter->data))->source = g_strdup(source);
        }
}

/* Get the HTML source from the table */
gchar *source_table_get(GtkHTML *html) {
    g_return_val_if_fail(html != NULL, NULL);
    
    GSList *iter;
    for(iter = source_table; iter != NULL; iter = g_slist_next(iter))
        if(((struct source_entry *)(iter->data))->html == html)
            return ((struct source_entry *)(iter->data))->source;
    return NULL;
}

/* Create a GtkHTML widget and do all the standard stuff that we do to all our
GtkHTML widgets */
GtkWidget *create_html(gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
    GtkWidget *html = gtk_html_new();
    g_signal_connect(html, "url_requested", G_CALLBACK(on_url_requested), NULL);
    g_signal_connect(html, "link_clicked", G_CALLBACK(on_link_clicked), NULL);
    g_signal_connect(html, "destroy", G_CALLBACK(on_html_destroy), NULL);
    source_table_add(GTK_HTML(html), "");
    update_font_size(html);
    return html;
}

/* Have the html widget display the HTML file in filename */
void html_load_file(GtkHTML *html, const gchar *filename) {
    g_return_if_fail(html != NULL);
    g_return_if_fail(filename != NULL || strlen(filename));
    
    GError *err = NULL;
    gchar *buf;

    /* Set the base path for relative paths in the HTML file */
    gchar *newbase = get_base_name(filename);
    gtk_html_set_base(html, newbase);
    g_free(newbase);

    /* Open a stream and write the contents of the file to it */
    GtkHTMLStream *stream = gtk_html_begin(html);
    if(g_file_get_contents(filename, &buf, NULL, &err)) {
        source_table_update(html, buf);
        gtk_html_write(html, stream, buf, strlen(buf));
        gtk_html_end(html, stream, GTK_HTML_STREAM_OK);
        g_free(buf);
        return;
    }
    gtk_html_end(html, stream, GTK_HTML_STREAM_ERROR);
    error_dialog(NULL, err, "Error opening HTML file '%s': ", filename);
}

/* Blank the GtkHTML widget */
void html_load_blank(GtkHTML *html) {
    g_return_if_fail(html != NULL);
    
    GtkHTMLStream *stream = gtk_html_begin(html);
    gtk_html_end(html, stream, GTK_HTML_STREAM_OK);
    source_table_update(html, "");
}

/* Reload the current page from the "cache" */
void html_refresh(GtkHTML *html) {
    g_return_if_fail(html != NULL);
    
    GtkHTMLStream *stream = gtk_html_begin(html);
    gchar *buf = source_table_get(html);
    gtk_html_write(html, stream, buf, strlen(buf));  
    gtk_html_end(html, stream, GTK_HTML_STREAM_OK);
}

/* This is the function responsible for getting the data from the URLs. There is
already a stream opened at this point, so we do not handle anything that does
not involve data being written to the GtkHTML widget. That should already have
been done in on_link_clicked. */
void on_url_requested(GtkHTML *html, const gchar *url, GtkHTMLStream *handle,
gpointer data) {
    g_return_if_fail(html != NULL);
    g_return_if_fail(url != NULL);
    g_return_if_fail(handle != NULL);
    
    GError *err = NULL;
    gchar *buf;
    gchar *anchor;
    gsize length;
    gchar *file = g_strdup(url);
    
    /* Get the anchor if there is a # in the URL, and remove it from the URL */
    if((anchor = strchr(file, '#')))
        *(anchor++) = '\0';

    /* inform: protocol can mean a file in any one of several locations;
    find it and save it in the variable "file" */
    if(g_str_has_prefix(url, "inform:/")) {
        gchar *real_file = NULL;
        if(check_datafile(url + 8))
            real_file = get_datafile_path(url + 8);
        
        gchar *checkfile = g_build_filename("doc", url + 8, NULL);
        if(check_datafile(checkfile))
            real_file = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename("doc", "doc_images", url + 8, NULL);
        if(check_datafile(checkfile))
            real_file = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename("doc", "sections", url + 8, NULL);
        if(check_datafile(checkfile))
            real_file = get_datafile_path(checkfile);
        g_free(checkfile);
        
        if(!real_file) {
            error_dialog(NULL, NULL, "Error opening file %s.", url + 8);
            gtk_html_end(html, handle, GTK_HTML_STREAM_ERROR);
            return;
        }
        
        g_free(file);
        file = g_strdup(real_file);
        g_free(real_file);
    }
    
    /* Open "file" and write it to the html widget */
    if(!g_file_get_contents(file, &buf, &length, &err)) {
        error_dialog(NULL, err, "Error opening file %s: ", file);
        gtk_html_end(html, handle, GTK_HTML_STREAM_ERROR);
        g_free(file);
        return;
    }
    gtk_html_write(html, handle, buf, length);
    gtk_html_end(html, handle, GTK_HTML_STREAM_OK);
    g_free(buf);
    g_free(file);
    return;
}

/* This function is called when the user clicks on a link. It handles the
different protocols used by the Inform help browser and, if necessary,
eventually passes control to on_url_requested */
void on_link_clicked(GtkHTML *html, const gchar *requested_url, gpointer data) {
    g_return_if_fail(html != NULL);
    g_return_if_fail(requested_url != NULL);
    
    GError *err = NULL;
    gchar *anchor;
    gchar *real_url = NULL;
    gchar *url = g_strdup(requested_url); /* make a copy so we can mess w/ it */
    
    /* Check if there is an anchor we have to jump to later and remove it from
    the regular URL */
    if((anchor = strchr(url, '#'))) {
        *(anchor++) = '\0';
        if(!strcmp(url, gtk_html_get_base(html))) {
            /* if we are only jumping to an anchor on the same page */
            gtk_html_jump_to_anchor(html, anchor);
            g_free(url);
            return;
        }
    }
    
    /* If there is no ':' (no protocol) then it is just a file */
    if(!strchr(url, ':')) {
        real_url = g_strdup(url);
    /* file: protocol */
    } else if(g_str_has_prefix(url, "file:")) {
        real_url = g_strdup(url + 5); /* Shove the pointer past the prefix */
    /* inform: protocol can mean one of several locations */
    } else if(g_str_has_prefix(url, "inform://Extensions")) {
        real_url = g_build_filename(g_get_home_dir(), ".wine", "drive_c",
          "Inform", "Documentation", url + 19, NULL);
    } else if(g_str_has_prefix(url, "inform:/")) {
        if(check_datafile(url + 8))
            real_url = get_datafile_path(url + 8);
        
        gchar *checkfile = g_build_filename("doc", url + 8, NULL);
        if(check_datafile(checkfile))
            real_url = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename("doc", "doc_images", url + 8, NULL);
        if(check_datafile(checkfile))
            real_url = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename("doc", "sections", url + 8, NULL);
        if(check_datafile(checkfile))
            real_url = get_datafile_path(checkfile);
        g_free(checkfile);
        
        checkfile = g_build_filename(g_get_home_dir(), ".wine", "drive_c", "Inform",
          "Documentation", url + 8, NULL);
        if(g_file_test(checkfile, G_FILE_TEST_EXISTS))
            real_url = g_strdup(checkfile);
        g_free(checkfile);
        
        if(!real_url) {
            g_free(url);
            return;
        }
        
        /* When clicking on a link to the documentation, this is a hack to
        get it to open in the documentation tab */
        int right = get_current_notebook(GTK_WIDGET(html));
        html = GTK_HTML(lookup_widget(GTK_WIDGET(html), 
          right? "docs_r" : "docs_l"));
        gtk_notebook_set_current_page(get_notebook(GTK_WIDGET(html), right),
          TAB_DOCUMENTATION);
    /* http: protocol, open in the default web browser */    
    } else if(g_str_has_prefix(url, "http:")
      || g_str_has_prefix(url, "mailto:")) {
        if(!gnome_url_show(url, &err)) {
            error_dialog(NULL, err, "Error opening external viewer for %s: ",
              url);
        }
        g_free(url);
        return;
    /* javascript: protocol, damned if I'm going to write an entire javascript
    interpreter, so here we just recognize the functions that are used in the
    Inform docs */
    } else if(g_str_has_prefix(url, "javascript:")) {
        gchar *function_call = g_strdup(url + 11);
        /* Handle the function pasteCode('...') which pastes its argument
        directly into the source */
        if(g_str_has_prefix(function_call, "pasteCode('")) {
            *(strrchr(function_call, '\'')) = '\0';
            paste_code(get_story(GTK_WIDGET(html))->buffer, function_call + 11);
            g_free(function_call);
            g_free(url);
            return;
        }
        /* Now the name of the function is something else, like "pasteCode134()"
        and we need to look the function body up in the source cache in order to
        find what text to paste */
        gchar *source = source_table_get(html);
        gchar *code = javascript_find_paste_code(source, function_call);
        paste_code(get_story(GTK_WIDGET(html))->buffer, code);
        g_free(code);
        g_free(function_call);
        g_free(url);
        return;
    /* source: protocol, a link to somewhere in the source file */
    } else if(g_str_has_prefix(url, "source:")) {
        gint line;
        if(sscanf(anchor, "line%d", &line))
            jump_to_line(GTK_WIDGET(html), line);
        g_free(url);
        return;
    } else {
        fprintf(stderr, "Unrecognized protocol: %s\n", url);
        g_free(url);
        return;
    }

    GtkHTMLStream *handle = gtk_html_begin(html);
    gchar *newbase = get_base_name(real_url);
    gtk_html_set_base(html, newbase);
    g_free(newbase);
    
    /* Read the file and store the contents in the source table */
    gchar *buf;
    gsize length;
    if(!g_file_get_contents(real_url, &buf, &length, &err)) {
        error_dialog(NULL, err, "Error opening file %s: ", real_url);
        gtk_html_end(html, handle, GTK_HTML_STREAM_ERROR);
        g_free(url);
        g_free(real_url);
        return;
    }
    source_table_update(html, buf);
    g_free(buf);
    
    /* Do the actual loading of the file into the html widget */
    on_url_requested(html, real_url, handle, data);
    /* Jump to an anchor if there is one */
    if(anchor)
        gtk_html_jump_to_anchor(html, anchor);
    g_free(real_url);
    g_free(url);
}

/* Destructor which removes the entry from the source table */
void on_html_destroy(GtkObject *widget, gpointer data)
{
    source_table_remove(GTK_HTML(widget));
}

/* Get the base path of an URL, for relative paths */
gchar *get_base_name(const gchar *url) {
    g_return_val_if_fail(url != NULL, NULL);
    
    gchar *newbase = g_malloc(strlen(url));
    strcpy(newbase, "");

    /* here we realize a minuscule improvement in performance by removing
    redundant directory changes in the base URL; oh, and also we remove the
    filename from the end */
    gchar **parts = g_strsplit(url, G_DIR_SEPARATOR_S, 0);
    gchar **ptr;
    for(ptr = parts ; *(ptr + 1) ; ptr++) { /*do not copy the last element*/
        if(strcmp(*ptr, "..")
          && strcmp(*ptr, ".")
          && !strcmp(*(ptr + 1), ".."))
            /* (if this element is not '.' or '..' and the next one is '..') */
            ptr++; /* skip them */
        else {
            g_strlcat(newbase, *ptr, strlen(url));
            g_strlcat(newbase, G_DIR_SEPARATOR_S, strlen(url));
        }
    }
    g_strfreev(parts);
    return newbase;
}

/* Find the code to be pasted within one of the pasteCode134, etc. javascript
functions */
gchar *javascript_find_paste_code(const gchar *source,
const gchar *function_call) {
    g_return_val_if_fail(source != NULL, NULL);
    g_return_val_if_fail(function_call != NULL, NULL);
    
    gchar *retval;
    gchar *function_name = g_strdup(function_call);
    
    /* erase everything after the first parenthesis as arguments */
    gchar *args;
    if((args = strchr(function_name, '(')))
        *args = '\0';

    gchar *buf = g_strdup(source);
    gchar *beginptr = strstr(buf, "<script language=\"JavaScript\">");
    if(beginptr == NULL)
        return NULL;
    beginptr += strlen("<script language=\"JavaScript\">");
    gchar *endptr = strstr(beginptr, "</script>");
    ptrdiff_t length = endptr - beginptr;
    gchar *result = g_strndup(beginptr, length);
    
    if(strstr(result, function_name)) {
        gchar *temp = g_strdup(strstr(result, "pasteCode('")
          + strlen("pasteCode('"));
        *(strstr(temp, "');")) = '\0';
        retval = g_strcompress(temp);
        g_free(temp);
    } else
        retval = javascript_find_paste_code(endptr, function_call);
    
    g_free(result);
    g_free(function_name);
    g_free(buf);
    return retval;
}