#include "config.h"
#ifdef ENABLE_CMD_PS
#include "custom_header.h"

void cmd_ps(int argc, char **argv) {
    int show_all = 0, detailed_format = 0;

    optind = 0; // getopt 상태 초기화

    // 옵션 처리
    int n;
    while ((n = getopt(argc, argv, "ef")) != -1) {
        switch (n) {
            case 'e':
                show_all = 1;			//모든 프로세스 표시
                break;
            case 'f':
                detailed_format = 1;	// 자세한 출력
                break;
            default:
                printf("Usage: ps [-e] [-f]\n");
                return;
        }
    }

    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

	// 자세한 출력(-f)
        // 자세한 출력(-f)
    if (detailed_format) {
        printf("%-10s %6s %6s %-6s %-10s %8s %s\n",
               "UID", "PID", "PPID", "STIME", "TTY", "TIME", "CMD");
    } else {
        printf("%7s %-7s %10s %-20s\n", "PID", "TTY", "TIME", "CMD");
    }

	// PID 디렉토리 탐색
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0])) {
            continue; 					// 숫자로 된  디렉토리만 처리
        }

        int pid = atoi(entry->d_name);

        // /proc/[pid]/stat 파일의 경로 생성
        char path[MAX_PATH], line[1024];
        snprintf(path, MAX_PATH, "/proc/%d/stat", pid);
        FILE *file = fopen(path, "r");
        if (file == NULL) {
            continue; 		// 파일을 열 수 없는 경우 건너뜀
        }

        if (fgets(line, sizeof(line), file) == NULL) {
            fclose(file);
            continue; 		// 읽기 실패 시 건너뜀
        }
        fclose(file);

        // 필요한 정보 파싱
        int ppid, tty_nr = 0;
        unsigned long utime, stime_val;
        unsigned long start_time;
		char comm[256];

        if (sscanf(line, "%d %255s %*c %d %d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %lu",
                   &pid, comm, &ppid, &tty_nr, &utime, &stime_val, &start_time) != 7) {
            continue;
        }

        // 세션 필터링 (-e 옵션 없으면 현재 세션만)
        if (!show_all && getsid(pid) != getsid(0)) {
            continue;
        }

		// UID, 사용자 이름 가져오기 =
		struct stat proc_stat;
		snprintf(path, MAX_PATH, "/proc/%d", pid);
		if (stat(path, &proc_stat) == -1) {
			continue;
		}
		struct passwd *user = getpwuid(proc_stat.st_uid);
		char *uid_name = user ? user->pw_name : "UNKNOWN";

        // 괄호 제거
        size_t len = strlen(comm);
        if (len > 2) {
            comm[len - 1] = '\0'; 				// 오른쪽 괄호 제거
            memmove(comm, comm + 1, len - 1); 	// 왼쪽 괄호 제거
        }

        // TTY 이름 가져오기
        char tty[20];
        if (tty_nr == 0) {
            strcpy(tty, "?");
        } else {
            snprintf(tty, sizeof(tty), "pts/%d", tty_nr & 0xFF);
        }

		// 시작 시간 (STIME)
		time_t boot_time;
		struct sysinfo info;
		sysinfo(&info);
		boot_time = time(NULL) - info.uptime;

		time_t proc_start_time = boot_time + start_time / sysconf(_SC_CLK_TCK);
		struct tm *stime_tm = localtime(&proc_start_time);
		char start_time_str[10];
		strftime(start_time_str, sizeof(start_time_str), "%H:%M", stime_tm);

        // CPU 시간 계산
        long total_time = (utime + stime_val) / sysconf(_SC_CLK_TCK);
        int hours = total_time / 3600;
        int minutes = (total_time % 3600) / 60;
        int seconds = total_time % 60;

        char time_str[12];
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);

        // /proc/[pid]/cmdline 파일의 경로 생성
        snprintf(path, MAX_PATH, "/proc/%d/cmdline", pid);
        file = fopen(path, "r");
        if (file == NULL) {
            continue; 					// cmdline 파일을 열 수 없는 경우 건너뜀
        }

        char cmdline[256];
        if (fgets(cmdline, sizeof(cmdline), file) == NULL || cmdline[0] == '\0') {
            strcpy(cmdline, comm); 		// cmdline이 비어 있으면 COMMAND 사용
        }
        fclose(file);

        // 출력
        if (detailed_format) {
            printf("%-10s %6d %6d %-6s %-10s %8s %s\n",
               uid_name, pid, ppid, start_time_str, tty, time_str, cmdline);
        } else {
            printf("%7d %-7s %10s %-20s\n", pid, tty, time_str, comm);
        }
    }

    optind = 0; // getopt 상태 초기화
    closedir(dir);
}


#endif
