# include "header.h"

pid_t backend_pid;

static GActionEntry app_actions[] = {
  { "new_directory", on_new_directory_action, NULL, NULL, NULL },
  { "rename", on_rename_action, NULL, NULL, NULL },
  { "delete", on_delete_action, NULL, NULL, NULL },
  { "move", on_move_action, NULL, NULL, NULL },
  { "copy", on_copy_action, NULL, NULL, NULL },
  { "chmod", on_chmod_action, NULL, NULL, NULL },
  { "add_favorite", on_add_to_favorites_action, NULL, NULL, NULL },
};

void start_backend() {
    backend_pid = fork();
    if (backend_pid == 0) {  // 자식 프로세스
        execl("./_build/a.out", "_build/a.out", NULL);
        exit(1);  // exec 실패 시
    }
}

static void app_activate (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  gtk_window_present (gtk_application_get_active_window(app));
}

static void app_startup (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);

  GtkBuilder *build = gtk_builder_new_from_file ("column.ui");
  GtkWidget *win = GTK_WIDGET (gtk_builder_get_object (build, "win"));
  GtkDirectoryList *directorylist = GTK_DIRECTORY_LIST (gtk_builder_get_object (build, "directorylist"));
  GtkButton *load_button = GTK_BUTTON(gtk_builder_get_object(build, "load_button"));
  GtkEntry *path_entry = GTK_ENTRY(gtk_builder_get_object(build, "path_entry"));
  GtkColumnView *columnview = GTK_COLUMN_VIEW(gtk_builder_get_object(build, "columnview"));
  GtkButton *upper_button = GTK_BUTTON(gtk_builder_get_object(build, "upper_button"));

  // UserData 구조체를 할당
  UserData *data = g_new0(UserData, 1);
  data->directorylist = directorylist;
  data->path_entry = path_entry;
  data->columnview = columnview;
  data->selected_file_info = NULL;
  data->main_window = GTK_WINDOW(win);
  data->entry = NULL;
  data->is_backend_done = FALSE;
  data->favorites_list = GTK_LIST_BOX(gtk_builder_get_object(build, "favorites_list"));
  update_favorites_list(data);

  g_signal_connect(data->favorites_list, "row-activated", G_CALLBACK(on_favorites_row_activated), data);
  update_favorites_list(data);

  // 시작 디렉토리 설정
  const char *start_path = "/tmp/test";
// 디렉토리 존재 여부 확인
  GFile *start_dir = g_file_new_for_path(start_path);
  if (!g_file_query_exists(start_dir, NULL)) {
    // 디렉토리가 존재하지 않으면 생성
    GError *error = NULL;
    if (!g_file_make_directory(start_dir, NULL, &error)) {
      if (error) {
        g_printerr("Error creating directory %s: %s\n", start_path, error->message);
        g_error_free(error);
      }
      // 디렉토리 생성 실패 시 현재 위치로  대체
      start_path = ".";
    }
  }

  // path_entry에 시작 경로 설정
  gtk_editable_set_text(GTK_EDITABLE(path_entry), start_path);

  // DirectoryList에 시작 경로 설정
  GFile *file = g_file_new_for_path(start_path);
  gtk_directory_list_set_file(directorylist, file);
  g_object_unref(file);

  // 액션 추가(app.new_directory)
  g_action_map_add_action_entries(G_ACTION_MAP(app), app_actions, G_N_ELEMENTS(app_actions), data);

  // "GtkEventControllerButton"를 GtkGestureClick으로 변경
  GtkWidget *scrolled_window = gtk_widget_get_parent(GTK_WIDGET(columnview));

  // 우클릭 제스처 설정
  GtkGesture *click_gesture = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_gesture), GDK_BUTTON_SECONDARY);
  gtk_widget_add_controller(GTK_WIDGET(columnview), GTK_EVENT_CONTROLLER(click_gesture));
  g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_button_pressed), data);

  // Selection 모델 가져오기 및 핸들러 연결
  GListModel *model = G_LIST_MODEL(directorylist);
  if (!model) {
      g_printerr("Error: Could not find 'columnview' in column.ui\n");
      return;
  }

  // SingleSelection 모델 생성
  GtkSelectionModel *single_sel = gtk_column_view_get_model(columnview);
  if (!single_sel) {
    g_printerr("Error: Could not create single selection model\n");
    return;
  }

  // Selection 모델에 시그널 연결
  g_signal_connect(single_sel, "selection-changed", G_CALLBACK(on_selection_changed), data);

  // 기본 경로를 현재 디렉토리로 설정
  gtk_editable_set_text(GTK_EDITABLE(path_entry), start_path);

  // update_directory 콜백을 data를 통해 연결
  g_signal_connect(load_button, "clicked", G_CALLBACK(update_directory), data);

  // 상위로 버튼을 클릭한 경우
  g_signal_connect(upper_button, "clicked", G_CALLBACK(upper_directory), data);

  // 디렉토리가 클릭한 경우
  g_signal_connect(columnview, "activate", G_CALLBACK(on_columnview_activate), data);

  g_object_unref (build);

  gtk_window_set_application (GTK_WINDOW (win), app);
}


#define APPLICATION_ID "com.github.ChoYoungwon.System-Programming-by-C"

int main (int argc, char **argv) {
  start_backend();
  GtkApplication *app;
  int stat;

  app = gtk_application_new (APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect (app, "startup", G_CALLBACK (app_startup), NULL);
  g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
  g_signal_connect (app, "shutdown", G_CALLBACK (on_application_shutdown), NULL);

  stat =g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
  return stat;
}

