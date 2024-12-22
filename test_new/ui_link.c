#include "header.h"

void send_favorite_message(const char *src_path, const char *dest_path) {
    struct mymsgbuf buf;
    key_t key;
    int msgid;

    key = ftok("command_keyfile", 1);
    if (key == -1) {
        perror("ftok");
        return;
    }

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        return;
    }

    buf.mtype = 8;  // 심볼릭 링크용 메시지 타입
    char message[512];
    snprintf(message, sizeof(message), "%s$%s", src_path, dest_path);
    strncpy(buf.mtext, message, sizeof(buf.mtext)-1);
    buf.mtext[sizeof(buf.mtext)-1] = '\0';

    if (msgsnd(msgid, &buf, sizeof(buf.mtext), 0) == -1) {
        perror("msgsnd failed");
    } else {
        printf("Symlink request sent: %s\n", message);
    }
}

void on_add_to_favorites_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    UserData *data = user_data;
    if (!data->selected_file_info) {
        g_printerr("No file selected to add to favorites.\n");
        return;
    }

    const char *file_name = g_file_info_get_name(data->selected_file_info);
    const char *current_path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));
    char *src_path = g_build_filename(current_path, file_name, NULL);
    char *link_path = g_build_filename("/tmp/link", file_name, NULL);

    data->is_backend_done = FALSE;
    send_favorite_message(src_path, link_path);
    
    g_free(src_path);
    g_free(link_path);
    
    start_backend_check(data);
}

void update_favorites_list(UserData *data) {
    GtkWidget *child;
    GDir *dir;
    const char *name;
    char *link_path;
    char target_path[PATH_MAX];
    ssize_t len;
    GtkWidget *box;
    GtkWidget *icon;
    GtkWidget *label;
    GtkWidget *row;

    // 기존 항목들 제거
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(data->favorites_list))) != NULL) {
        gtk_list_box_remove(data->favorites_list, child);
    }

    // /tmp/link 디렉토리 읽기
    dir = g_dir_open("/tmp/link", 0, NULL);
    if (!dir) return;

    while ((name = g_dir_read_name(dir))) {
        link_path = g_build_filename("/tmp/link", name, NULL);
        len = readlink(link_path, target_path, sizeof(target_path) - 1);
        
        if (len != -1) {
            target_path[len] = '\0';
            
            box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
            
            // 아이콘 추가
            icon = gtk_image_new_from_icon_name("document");
            gtk_box_append(GTK_BOX(box), icon);
            
            // 파일 이름 레이블
            label = gtk_label_new(name);
            gtk_widget_set_hexpand(label, TRUE);
            gtk_label_set_xalign(GTK_LABEL(label), 0);
            gtk_box_append(GTK_BOX(box), label);
            
            // 링크 정보 저장
            g_object_set_data_full(G_OBJECT(box), "target-path", 
                                 g_strdup(target_path), g_free);
            
            row = gtk_list_box_row_new();
            gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
            gtk_list_box_append(data->favorites_list, row);
        }
        
        g_free(link_path);
    }
    
    g_dir_close(dir);
}

// 즐겨찾기 항목 클릭 처리
void on_favorites_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    UserData *data = user_data;
    if (!row) return;

    GtkWidget *box_widget = gtk_list_box_row_get_child(row);
    const char *target_path = g_object_get_data(G_OBJECT(box_widget), "target-path");
    if (!target_path) return;

    char *dir_path = g_path_get_dirname(target_path);
    gtk_editable_set_text(GTK_EDITABLE(data->path_entry), dir_path);
    update(data);
    g_free(dir_path);
}
