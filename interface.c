//
// Created by oui on 21/06/23.
//
#include <gtk-3.0/gtk/gtk.h>
#include <stdio.h>
#include <stdio.h>

#include "interface.h"


void graphicZip(){
    GtkBuilder *builder;
    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "main.glade", NULL);

    windows = GTK_WIDGET(gtk_builder_get_object(builder, "windows"));

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);

    gtk_widget_show_all(windows);
    gtk_main();

}