#include "header.h"

// 위치 변경시 사용시 호출되는 콜백
void update_directory (GtkButton *button, gpointer user_data) {

  UserData *data = (UserData *)user_data;
  const char *path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));

  if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
    GFile *new_dir = g_file_new_for_path(path);
    gtk_directory_list_set_file(data->directorylist, new_dir);
    g_object_unref(new_dir);
  } else {
    g_printerr("Invalid directory path: %s\n", path);
  }
}

// 이전 경로로 돌아가는 콜백
void upper_directory (GtkButton *button, gpointer user_data) {
  UserData *data = (UserData *)user_data;

  // 현재 GtkEntry에서 경로 얻기
  const char *path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));

  // 현재 경로에 대한 GFile 생성
  GFile *current_dir = g_file_new_for_path(path);

  // 상위 디렉토리 GFile 얻기
  GFile *parent_dir = g_file_get_parent(current_dir);
  if (parent_dir) {
    // 상위 디렉토리가 존재하면 directorylist 갱신
    gtk_directory_list_set_file(data->directorylist, parent_dir);

    // 상위 디렉토리의 실제 경로를 얻어 GtkEntry 업데이트
    char *parent_path = g_file_get_path(parent_dir);
    gtk_editable_set_text(GTK_EDITABLE(data->path_entry), parent_path);

    g_free(parent_path);
    g_object_unref(parent_dir);
  } else {
    // 상위 디렉토리가 없을 경우(예: 루트 디렉토리)
    g_printerr("No parent directory.\n");
  }

  g_object_unref(current_dir);
}

void on_columnview_activate(GtkColumnView *view, guint position, gpointer user_data) {
   UserData *data = (UserData *)user_data;
   GListModel *model = G_LIST_MODEL(gtk_column_view_get_model(view));
   GFileInfo *info = G_FILE_INFO(g_list_model_get_item(model, position));
   
   if (!info)
     return;

   const char *name = g_file_info_get_name(info);
   const char *current_path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));
   
   // 전체 경로 생성
   char *full_path = g_build_filename(current_path, name, NULL);
   
   // 파일 유형에 따른 분기 처리
   switch (g_file_info_get_file_type(info)) {
     case G_FILE_TYPE_DIRECTORY:
       {
         GFile *cur_dir = g_file_new_for_path(current_path);
         GFile *new_dir = g_file_get_child(cur_dir, name);
         
         gtk_directory_list_set_file(data->directorylist, new_dir);
         
         char *new_path = g_file_get_path(new_dir);
         gtk_editable_set_text(GTK_EDITABLE(data->path_entry), new_path);
         
         g_free(new_path);
         g_object_unref(new_dir);
         g_object_unref(cur_dir);
       }
       break;

     case G_FILE_TYPE_REGULAR:
       {
         // 파일 확장자 확인
         const char *ext = strrchr(name, '.');
         
         if (ext) {
           // 텍스트 기반 파일 확장자 목록
           const char *text_extensions[] = {
             ".txt", ".c", ".h", ".py", ".cpp", ".java", 
             ".js", ".json", ".md", ".css", ".html", ".rs"
           };
           
           // 확장자 일치 여부 확인
           gboolean is_text_file = FALSE;
           for (size_t i = 0; i < G_N_ELEMENTS(text_extensions); i++) {
             if (g_str_has_suffix(name, text_extensions[i])) {
               is_text_file = TRUE;
               break;
             }
           }
           
           if (is_text_file) {
             // gedit으로 파일 열기
             char *command = g_strdup_printf("x-terminal-emulator -e nano '%s' &", full_path);
             system(command);
             g_free(command);
           }
         }
       }
       break;

     default:
       g_print("Unsupported file type\n");
       break;
   }
   
   // 메모리 해제
   g_free(full_path);
}

void on_move_dialog_response(GtkDialog *dialog, int response, gpointer user_data) {
    UserData *data = user_data;
    
    if (response == GTK_RESPONSE_OK && data->selected_file_info) {
        const char *dest_path = gtk_editable_get_text(GTK_EDITABLE(data->entry));
        const char *file_name = g_file_info_get_name(data->selected_file_info);
        const char *current_path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));

        if (!dest_path || !*dest_path) {
            g_printerr("Destination path is empty.\n");
            gtk_window_close(GTK_WINDOW(dialog));
            return;
        }

        // 원본 파일의 전체 경로
        char *src_path = g_build_filename(current_path, file_name, NULL);
        
        // 목적지 경로 구성 - 항상 대상 디렉토리와 파일 이름을 결합
        char *full_dest_path = g_build_filename(dest_path, file_name, NULL);

        g_print("Moving from: %s\n", src_path);
        g_print("Moving to: %s\n", full_dest_path);

        char *message = g_strdup_printf("%s$%s", src_path, full_dest_path);
        data->is_backend_done = FALSE;
        send_message(5, message, dialog);

        g_free(message);
        g_free(src_path);
        g_free(full_dest_path);
    }

    gtk_window_close(GTK_WINDOW(dialog));
    start_backend_check(data);
}

// 이동 기능 구현하는 콜백함
void on_move_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    UserData *data = user_data;
    if (!data->selected_file_info) {
        g_printerr("No file selected to move.\n");
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "파일 이동",
        GTK_WINDOW(data->main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR,
        "_취소", GTK_RESPONSE_CANCEL,
        "_이동", GTK_RESPONSE_OK,
        NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);

    const char *current_name = g_file_info_get_name(data->selected_file_info);
    char *label_text = g_strdup_printf("선택된 파일: %s", current_name);
    GtkWidget *info_label = gtk_label_new(label_text);
    gtk_widget_set_halign(info_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), info_label);
    g_free(label_text);

    GtkWidget *dest_label = gtk_label_new("이동할 경로:");
    gtk_widget_set_halign(dest_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), dest_label);

    GtkWidget *entry = gtk_entry_new();
    // gtk_entry_set_text를 gtk_editable_set_text로 변경
    gtk_editable_set_text(GTK_EDITABLE(entry), 
                         gtk_editable_get_text(GTK_EDITABLE(data->path_entry)));
    gtk_box_append(GTK_BOX(vbox), entry);
    gtk_box_append(GTK_BOX(content_area), vbox);
    data->entry = GTK_ENTRY(entry);

    gtk_widget_show(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(on_move_dialog_response), data);
}

void on_copy_dialog_response(GtkDialog *dialog, int response, gpointer user_data) {
    UserData *data = user_data;
    
    if (response == GTK_RESPONSE_OK && data->selected_file_info) {
        const char *dest_path = gtk_editable_get_text(GTK_EDITABLE(data->entry));
        const char *file_name = g_file_info_get_name(data->selected_file_info);
        const char *current_path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));

        if (!dest_path || !*dest_path) {
            g_printerr("Destination path is empty.\n");
            gtk_window_close(GTK_WINDOW(dialog));
            return;
        }

        // 원본 파일의 전체 경로
        char *src_path = g_build_filename(current_path, file_name, NULL);
        
        // 목적지 경로 구성 - 항상 대상 디렉토리와 파일 이름을 결합
        char *full_dest_path = g_build_filename(dest_path, file_name, NULL);

        g_print("Copying from: %s\n", src_path);
        g_print("Copying to: %s\n", full_dest_path);

        char *message = g_strdup_printf("%s$%s", src_path, full_dest_path);
        data->is_backend_done = FALSE;
        send_message(6, message, dialog);

        g_free(message);
        g_free(src_path);
        g_free(full_dest_path);
    }

    gtk_window_close(GTK_WINDOW(dialog));
    start_backend_check(data);
}

// 복사 기능 수행하는 콜백 함수
void on_copy_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    UserData *data = user_data;
    if (!data->selected_file_info) {
        g_printerr("No file selected to copy.\n");
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "파일 복사",
        GTK_WINDOW(data->main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR,
        "_취소", GTK_RESPONSE_CANCEL,
        "_복사", GTK_RESPONSE_OK,
        NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);

    const char *current_name = g_file_info_get_name(data->selected_file_info);
    char *label_text = g_strdup_printf("선택된 파일: %s", current_name);
    GtkWidget *info_label = gtk_label_new(label_text);
    gtk_widget_set_halign(info_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), info_label);
    g_free(label_text);

    GtkWidget *dest_label = gtk_label_new("복사할 경로:");
    gtk_widget_set_halign(dest_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), dest_label);

    GtkWidget *entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entry), 
                         gtk_editable_get_text(GTK_EDITABLE(data->path_entry)));
    gtk_box_append(GTK_BOX(vbox), entry);

    gtk_box_append(GTK_BOX(content_area), vbox);
    data->entry = GTK_ENTRY(entry);

    gtk_widget_show(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(on_copy_dialog_response), data);
}










