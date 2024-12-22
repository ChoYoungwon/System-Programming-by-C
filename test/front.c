#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

static GtkWidget *entry;
static GtkWidget *textview;

void send_command(const char *command, char *response) {
    if (strncmp(command, "LIST", 4) == 0) {
        snprintf(response, BUFFER_SIZE, "Mock response: Listing directory for %s", command + 5);
    } else if (strncmp(command, "MKDIR", 5) == 0) {
        snprintf(response, BUFFER_SIZE, "Mock response: Created directory %s", command + 6);
    } else {
        snprintf(response, BUFFER_SIZE, "Mock response: Unknown command");
    }
}

void on_list_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    const char *path = gtk_editable_get_text(GTK_EDITABLE(entry));
    char command[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    snprintf(command, BUFFER_SIZE, "LIST %s", path);
    send_command(command, response);

    gtk_text_buffer_set_text(buffer, response, -1);
}

void on_mkdir_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    const char *path = gtk_editable_get_text(GTK_EDITABLE(entry));
    char command[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    snprintf(command, BUFFER_SIZE, "MKDIR %s", path);
    send_command(command, response);

    gtk_text_buffer_set_text(buffer, response, -1);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Directory Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entry), "Enter directory path...");
    gtk_box_append(GTK_BOX(vbox), entry);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(vbox), button_box);

    GtkWidget *list_button = gtk_button_new_with_label("List Directory");
    g_signal_connect(list_button, "clicked", G_CALLBACK(on_list_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), list_button);

    GtkWidget *mkdir_button = gtk_button_new_with_label("Create Directory");
    g_signal_connect(mkdir_button, "clicked", G_CALLBACK(on_mkdir_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), mkdir_button);

    textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    gtk_box_append(GTK_BOX(vbox), textview);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.directorymanager", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
