#include "header.h"

// 메시지 전달 함수
void send_message(int buf_type, char *file_path, GtkDialog *dialog) {
    struct mymsgbuf buf;
    key_t key;
    int msgid;

    key = ftok("command_keyfile", 1);
    if (key == -1) {
        perror("ftok");
        g_free(file_path);
        gtk_window_close(GTK_WINDOW(dialog));
        return;
    }
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        g_free(file_path);
        gtk_window_close(GTK_WINDOW(dialog));
        return;
    }

    buf.mtype = buf_type;  // Message type, must be > 0
    strncpy(buf.mtext, file_path, sizeof(buf.mtext)-1);
    buf.mtext[sizeof(buf.mtext)-1] = '\0';  // Ensure null-termination

    size_t msg_len = strlen(file_path) + 1;
	printf("send : %s\n", buf.mtext);
    if (msgsnd(msgid, &buf, sizeof(buf.mtext), 0) == -1) {
        perror("msgsnd failed");
    } else {
        printf("Delete request sent for: %s\n", file_path);
    }
}

gboolean update(gpointer user_data) {
    UserData *data = (UserData *)user_data;
    const char *path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));
    
    if (!g_file_test(path, G_FILE_TEST_IS_DIR)) {
        g_printerr("Invalid directory path: %s\n", path);
        return FALSE;
    }

    // DirectoryList 로딩이 완료될 때까지 기다림
    GFile *new_dir = g_file_new_for_path(path);
    if (new_dir) {
        // 기존 모델을 새로 고침
        gtk_directory_list_set_file(data->directorylist, NULL);
        g_timeout_add(100, update_ui, data);  // 약간의 지연 후 새로운 파일 설정
        g_object_unref(new_dir);
    }

    return FALSE;
}

gboolean update_ui(gpointer user_data) {
    UserData *data = (UserData *)user_data;
    const char *path = gtk_editable_get_text(GTK_EDITABLE(data->path_entry));
    
    GFile *new_dir = g_file_new_for_path(path);
    if (!new_dir) {
        g_printerr("Failed to create GFile for path: %s\n", path);
        return FALSE;
    }

    // attributes 명시적 설정
    gtk_directory_list_set_attributes(data->directorylist, 
        "standard::name,standard::icon,standard::size,standard::type,time::modified");

    gtk_directory_list_set_file(data->directorylist, new_dir);
    g_object_unref(new_dir);

    g_print("Directory list updated for path: %s\n", path);
    return FALSE;
}

// 백엔드 메시지 수신 함수 수정
void on_backend_message_received(const char *message, gpointer user_data) {
    UserData *data = (UserData *)user_data;
    g_print("Message received(back): %s\n", message);

    if (strcmp(message, "success") == 0) {
        data->is_backend_done = TRUE;
        // 일반적인 UI 업데이트
        g_timeout_add(200, update, data);

		// 즐겨찾기 목록 업데이트
		g_timeout_add(300, (GSourceFunc)update_favorites_list, data);
        g_print("Update scheduled\n");
    } else {
        g_printerr("Unknown message: %s\n", message);
    }
}

// 메시지 큐에서 메시지 읽기
gboolean read_backend_message(gpointer user_data) {
    UserData *data = (UserData *)user_data;
    struct mymsgbuf mbuf;
	key_t key;
	int msgid;

	// 응답 메시지 큐 생성
    key_t response_key = ftok("response_keyfile", 1);
	if (response_key == -1) {
        perror("ftok (response)");
        return TRUE;
    }

    int response_msgid = msgget(response_key, IPC_CREAT | 0666);
    if (response_msgid == -1) {
        perror("msgget (response)");
        exit(1);
    }

    // 메시지 큐에서 메시지 읽기 (비차단)
    if (msgrcv(response_msgid, &mbuf, sizeof(mbuf.mtext), 10, IPC_NOWAIT) != -1) {
        // 메시지 수신 완료: 처리 함수 호출
			g_print("recived : %s\n", mbuf.mtext);
        	on_backend_message_received(mbuf.mtext, data);
	}

    return TRUE; // 계속 폴링
}

void start_backend_check(UserData *data) {
    static gboolean is_check_running = FALSE;
	// 타이머를 시작 (1초 간격으로 실행)
	if (!is_check_running) {
        g_timeout_add(1000, read_backend_message, data);
        is_check_running = TRUE;
    }
}


