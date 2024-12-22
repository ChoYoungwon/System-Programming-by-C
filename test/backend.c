#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <dirent.h>
#include <sys/stat.h>

#define MSG_QUEUE_KEY 1234
#define BUFFER_SIZE 256

struct message {
    long mtype;
    char mtext[BUFFER_SIZE];
};

void handle_command(const char *command, char *response) {
    if (strncmp(command, "LIST", 4) == 0) {
        const char *path = command + 5;
        DIR *dir = opendir(path);
        if (dir == NULL) {
            snprintf(response, BUFFER_SIZE, "ERROR: Could not open directory %s", path);
            return;
        }

        struct dirent *entry;
        response[0] = '\0'; // Clear response buffer
        while ((entry = readdir(dir)) != NULL) {
            strcat(response, entry->d_name);
            strcat(response, "\n");
        }
        closedir(dir);
    } else if (strncmp(command, "MKDIR", 5) == 0) {
        const char *path = command + 6;
        if (mkdir(path, 0755) == 0) {
            snprintf(response, BUFFER_SIZE, "SUCCESS: Directory %s created", path);
        } else {
            snprintf(response, BUFFER_SIZE, "ERROR: Could not create directory %s", path);
        }
    } else {
        snprintf(response, BUFFER_SIZE, "ERROR: Unknown command");
    }
}

int main() {
    int msgid;
    struct message msg;

    // 메시지 큐 생성
    msgid = msgget(MSG_QUEUE_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    printf("Backend is running...\n");

    while (1) {
        // 클라이언트로부터 메시지 수신
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
            perror("msgrcv failed");
            continue;
        }

        printf("Received: %s\n", msg.mtext);

        // 명령 처리
        char response[BUFFER_SIZE];
        handle_command(msg.mtext, response);

        // 클라이언트에 응답 전송
        msg.mtype = 2;  // 응답 메시지 타입
        strncpy(msg.mtext, response, BUFFER_SIZE);
        if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
            perror("msgsnd failed");
        }
    }

    return 0;
}
