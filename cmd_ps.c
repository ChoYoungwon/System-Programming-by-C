#include "custom_header.h"

void cmd_ps (int argc, char **argv)
{
	(void)argc;
	(void)argv;
	DIR *dir;
	struct dirent *entry;
	char path[MAX_PATH], line[1024];
	int pid;
	pid_t current_sid = getsid(0);	// 현재 세션 id 가져오기

	// 현재 터미널의 TTY를 가져온다
	dir = opendir("/proc");
	if (dir == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	printf("%7s %-7s %10s %-20s\n", "PID", "TTY", "TIME", "CMD");
	while((entry = readdir(dir)) != NULL) {
		// 파일명이 숫자인 경우만 사용합니다.
		if(!isdigit(entry->d_name[0])) {
			continue;
		}

		pid = atoi(entry->d_name);

		//프로세스의 세션 ID가 현재 세션  ID와 일치하는지 확인
		if (getsid(pid) != current_sid) {
			continue;
		}

		// /proc/pid/stat 파일의 경로 생성
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

		// 필요한 정보만 파싱: PID, COMMAND, STATE, UTIME, STIME, TTY_NR
	        int tty_nr;
	        unsigned long utime, stime;
	        char comm[256];
	        sscanf(line, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %d",
	               &pid, comm, &utime, &stime, &tty_nr);

	        // 괄호 제거: COMMAND는 괄호로 둘러싸여 있으므로 제거
	        comm[strlen(comm) - 1] = '\0';

	        // TTY 이름 가져오기
	        char tty[20];
	        if (tty_nr == 0) {
	            strcpy(tty, "?");
	        } else {
	            snprintf(tty, sizeof(tty), "pts/%d", tty_nr & 0xFF);
	        }

	        // CPU 시간 계산 (초 단위로 변환)
	        long total_time = (utime + stime) / sysconf(_SC_CLK_TCK);
	        int hours = total_time / 3600;
	        int minutes = (total_time % 3600) / 60;
	        int seconds = total_time % 60;

	        char time_str[12];
	        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);

	        // /proc/[pid]/cmdline 파일의 경로를 생성합니다.
	        snprintf(path, MAX_PATH, "/proc/%d/cmdline", pid);
	        file = fopen(path, "r");
	        if (file == NULL) {
	            continue;
	        }

	        char cmdline[256];
	        if (fgets(cmdline, sizeof(cmdline), file) == NULL || cmdline[0] == '\0') {
	            strcpy(cmdline, comm + 1);  // cmdline이 비어 있으면 COMMAND 사용
	        }
	        fclose(file);
	        printf("%7d %-7s %10s %-20s\n", pid, tty, time_str, cmdline);
	}

	closedir(dir);

}
