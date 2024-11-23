#include "custom_header.h"

void cmd_ps(int argc, char **argv) {
    int show_all = 0, specific_user = 0, show_without_tty = 0;
    char *user_name = NULL;

    // 옵션 처리
    int n;
	optind = 1;
    while ((n = getopt(argc, argv, "aux")) != -1) {
        switch (n) {
            case 'a':
                show_all = 1;
                break;
            case 'u':
                specific_user = 1;
                user_name = getenv("USER"); // 현재 사용자
                break;
            case 'x':
                show_without_tty = 1;
                break;
            default:
                printf("Usage: ps [-a] [-u] [-x]\n");
                return;
        }
    }

    DIR *dir;
    struct dirent *entry;
    int pid;
    pid_t current_sid = getsid(0); // 현재 세션 ID 가져오기

    dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    printf("%7s %-7s %10s %-20s\n", "PID", "TTY", "TIME", "CMD");
    while ((entry = readdir(dir)) != NULL) {
        // 파일명이 숫자인 경우만 사용
        if (!isdigit(entry->d_name[0])) {
            continue;
        }

        pid = atoi(entry->d_name);

        // /proc/pid/stat 파일의 경로 생성
		char path[MAX_PATH], line[1024];
        snprintf(path, MAX_PATH, "/proc/%d/stat", pid);
        FILE *file = fopen(path, "r");
        if (file == NULL) {
            continue;
        }

        if (fgets(line, sizeof(line), file) == NULL) {
            fclose(file);
            continue;
        }
        fclose(file);

        // 필요한 정보만 파싱
        int tty_nr;
        unsigned long utime, stime;
        char comm[256];
        sscanf(line, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %d",
               &pid, comm, &utime, &stime, &tty_nr);

        // 세션 필터링 (-a 옵션 없으면 세션 ID가 현재 세션과 같아야 함)
        if (!show_all && getsid(pid) != current_sid) {
            continue;
        }

        // 사용자 필터링 (-u 옵션)
        if (specific_user) {
            struct stat proc_stat;
            snprintf(path, MAX_PATH, "/proc/%d", pid);
            if (stat(path, &proc_stat) == -1) {
                continue;
            }
            struct passwd *user = getpwuid(proc_stat.st_uid);
            if (user == NULL || strcmp(user->pw_name, user_name) != 0) {
                continue;
            }
        }

        // TTY 필터링 (-x 옵션 없으면 TTY가 있어야 함)
        if (!show_without_tty && tty_nr == 0) {
            continue;
        }

        // 괄호 제거
        comm[strlen(comm) - 1] = '\0';

        // TTY 이름 가져오기
        char tty[20];
        if (tty_nr == 0) {
            strcpy(tty, "?");
        } else {
            snprintf(tty, sizeof(tty), "pts/%d", tty_nr & 0xFF);
        }

        // CPU 시간 계산
        long total_time = (utime + stime) / sysconf(_SC_CLK_TCK);
        int hours = total_time / 3600;
        int minutes = (total_time % 3600) / 60;
        int seconds = total_time % 60;

        char time_str[12];
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);

        // /proc/pid/cmdline 파일의 경로 생성
        snprintf(path, MAX_PATH, "/proc/%d/cmdline", pid);
        file = fopen(path, "r");
        if (file == NULL) {
            continue;
        }

        char cmdline[256];
        if (fgets(cmdline, sizeof(cmdline), file) == NULL || cmdline[0] == '\0') {
            strcpy(cmdline, comm + 1);
        }
        fclose(file);

        // 출력
        printf("%7d %-7s %10s %-20s\n", pid, tty, time_str, cmdline);
    }

    closedir(dir);
}
