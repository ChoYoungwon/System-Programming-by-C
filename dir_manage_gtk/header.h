#include <gtk/gtk.h>
#include <string.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
  GtkDirectoryList *directorylist;
  GtkEntry *path_entry;
  GFileInfo *selected_file_info;
  GtkColumnView *columnview;
  GtkWindow *main_window;
  GtkEntry *entry;
  gboolean is_backend_done;
  GtkListBox *favorites_list;
} UserData;

struct mymsgbuf {
    long mtype;
    char mtext[256];  // 경로 길이에 맞춰 조정 가능
};

// ui_list 부분 함수 선언
GIcon * get_icon_factory (GtkListItem *item, GFileInfo *info);
char * get_file_name_factory (GtkListItem *item, GFileInfo *info);
goffset get_file_size_factory (GtkListItem *item, GFileInfo *info);
char * get_file_time_modified_factory (GtkListItem *item, GFileInfo *info);
char * get_file_name (GFileInfo *info);
goffset get_file_size (GFileInfo *info);
gint64 get_file_unixtime_modified (GFileInfo *info);

//ui_cd 부분 함수 선언
void update_directory (GtkButton *button, gpointer user_data);
void upper_directory (GtkButton *button, gpointer user_data);
void on_columnview_activate(GtkColumnView *view, guint position, gpointer user_data);
void on_copy_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_copy_dialog_response(GtkDialog *dialog, int response, gpointer user_data);
void on_move_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_move_dialog_response(GtkDialog *dialog, int response, gpointer user_data);

//ui_manage 부분 함수 선언
void show_popup_menu(GtkWidget *parent_widget, double x, double y, gpointer user_data);
void on_new_directory_action (GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_selection_changed(GtkSingleSelection *single_sel, guint position, guint n_items, gpointer user_data);
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void on_rename_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_delete_response(GtkDialog *dialog, int response, gpointer user_data);
void on_delete_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_button_pressed(GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer user_data);
void on_application_shutdown(GApplication *app, gpointer user_data);
void on_chmod_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void on_chmod_dialog_response(GtkDialog *dialog, int response, gpointer user_data);

//backend, ui 통신 함수 선언
void send_message(int buf_type, char *file_path, GtkDialog *dialog);
void on_backend_message_received(const char *message, gpointer user_data);
gboolean read_backend_message(gpointer user_data);
void start_backend_check(UserData *data);
gboolean update_ui(gpointer user_data);
gboolean update(gpointer user_data);

//ui_link 부분 함수 선언
void on_add_to_favorites_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void update_favorites_list(UserData *data);
void on_favorites_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data);
void send_favorite_message(const char *src_path, const char *dest_path);










