#include "custom_header.h"

void cmd_ps(int argc, char **argv) {
    int show_all = 0, specific_user = 0, show_without_tty = 0;
    char *user_name = NULL;

    optind = 1; // getopt 상태 초기화
    opterr = 0;
    optopt = 0;

    // 옵션 처리
    int opt;
    while ((opt = getopt(argc, argv, "aux")) != -1) {
        switch (opt) {
            case 'a':
                show_all = 1;
                break;
            case 'u':
                specific_user = 1;
                user_name = getenv("USER"); // 현재 사용자 이름
                break;
            case 'x':
                show_without_tty = 1;
                break;
            default:
                printf("Usage: ps [-a] [-u] [-x]\n");
                return;
        }
    }

    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    printf("%7s %-7s %10s %-20s\n", "PID", "TTY", "TIME", "CMD");

    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0])) {
            continue; // PID 디렉토리만 처리
        }

        int pid = atoi(entry->d_name);

        // /proc/[pid]/stat 파일의 경로 생성
        char path[MAX_PATH], line[1024];
        snprintf(path, MAX_PATH, "/proc/%d/stat", pid);
        FILE *file = fopen(path, "r");
        if (file == NULL) {
            continue; // 파일을 열 수 없는 경우 건너뜀
        }

        if (fgets(line, sizeof(line), file) == NULL) {
            fclose(file);
            continue; // 읽기 실패 시 건너뜀
        }
        fclose(file);

        // 필요한 정보 파싱
        int tty_nr;
        unsigned long utime, stime;
        char comm[256];
        if (sscanf(line, "%d %255s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %d",
                   &pid, comm, &utime, &stime, &tty_nr) != 5) {
            continue; // 파싱 실패 시 건너뜀
        }

        // 세션 필터링 (-a 옵션 없으면 현재 세션만)
        if (!show_all && getsid(pid) != getsid(0)) {
            continue;
        }

        // 사용자 필터링 (-u 옵션)
        if (specific_user) {
            struct stat proc_stat;
            snprintf(path, MAX_PATH, "/proc/%d", pid);
            if (stat(path, &proc_stat) == -1) {
                continue; // 파일 정보를 가져올 수 없는 경우 건너뜀
            }
            struct passwd *user = getpwuid(proc_stat.st_uid);
            if (user == NULL || strcmp(user->pw_name, user_name) != 0) {
                continue; // 사용자 이름이 다를 경우 건너뜀
            }
        }

        // TTY 필터링
        if (!show_without_tty && tty_nr == 0) {
            continue;
        }

        // 괄호 제거: COMMAND는 괄호로 둘러싸여 있으므로 제거
        size_t len = strlen(comm);
        if (len > 2) {
            comm[len - 1] = '\0'; // 오른쪽 괄호 제거
            memmove(comm, comm + 1, len - 1); // 왼쪽 괄호 제거
        }

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

        // /proc/[pid]/cmdline 파일의 경로 생성
        snprintf(path, MAX_PATH, "/proc/%d/cmdline", pid);
        file = fopen(path, "r");
        if (file == NULL) {
            continue; // cmdline 파일을 열 수 없는 경우 건너뜀
        }

        char cmdline[256];
        if (fgets(cmdline, sizeof(cmdline), file) == NULL || cmdline[0] == '\0') {
            strcpy(cmdline, comm); // cmdline이 비어 있으면 COMMAND 사용
        }
        fclose(file);

        // 출력
        printf("%7d %-7s %10s %-20s\n", pid, tty, time_str, cmdline);
    }

    optind = 1; // getopt 상태 초기화
    opterr = 0;
    optopt = 0;

    closedir(dir);
}
