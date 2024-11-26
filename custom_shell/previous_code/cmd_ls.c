#include "custom_header.h"

const char *get_type_str(char type)
{
	switch (type) {
	case DT_BLK:
            return "b";			// 블록 장치
        case DT_CHR:
            return "c";			// 문자 장치
        case DT_DIR:
            return "d";			// 디렉토리
        case DT_FIFO:
            return "p";			// FIFO/파이프
        case DT_LNK:
            return "l";			// 심볼릭 링크
        case DT_REG:
            return "-";			// 일반 파일
        case DT_SOCK:
            return "s";			// 소켓
        default:
            return "u";			// 알 수 없는 타입
	}
}

/* 파일의 접근 권한을 rwx 형태로 출력하는 함수 */
void print_permissions(mode_t mode) {
	char perms[10];
	perms[0] = (mode & S_IRUSR) ? 'r' : '-';
	perms[1] = (mode & S_IWUSR) ? 'w' : '-';
	// setuid 비트 적용 여부
	perms[2] = (mode & S_IXUSR) ? ((mode & S_ISUID) ? 's' : 'x') : ((mode & S_ISUID) ? 'S' : '-');
	perms[3] = (mode & S_IRGRP) ? 'r' : '-';
	perms[4] = (mode & S_IWGRP) ? 'w' : '-';
	// setgid 비트 적용 여부
	perms[5] = (mode & S_IXGRP) ? ((mode & S_ISGID) ? 's' : 'x') : ((mode & S_ISGID) ? 'S' : '-');
	perms[6] = (mode & S_IROTH) ? 'r' : '-';
	perms[7] = (mode & S_IWOTH) ? 'w' : '-';
	// sticky bit 적용 여부
	perms[8] = (mode & S_IXOTH) ? ((mode & S_ISVTX) ? 't' : 'x') : ((mode & S_ISVTX) ? 'T' : '-');
	perms[9] = '\0';

	printf("%s ", perms);
}

/*심벌릭 링크 내용 읽고 출력하는  함수*/
void read_symbolic(char *path)
{
	char buf[BUFSIZ];
	const int num = strlen(BASE_PATH);
	int n;
	n = readlink(path, buf, BUFSIZ);
	if (n == -1) {
		perror("readlink");
		exit(1);
	}
	buf[n] = '\0';
	printf("-> %s\n", (buf+num));
}



void cmd_ls(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	DIR *dp;
	struct dirent *dent;
	struct stat file_stat;
	struct passwd *owner;
	struct group *group;
	char *current_dir;
	char *file_path = (char *)malloc(100 * sizeof(char));
	char buffer[BUFSIZ];
	if (file_path == NULL) {
		perror("malloc");
		exit(1);
	}
	current_dir = getcwd(NULL, BUFSIZ);
	/* 현재 디렉토리의 하위 폴더를 dp 구조체 변수로 저장 */
	dp = opendir(current_dir);

	while((dent = readdir(dp))) {
		strcpy(file_path, current_dir);
		strcat(file_path, "/");
		strcat(file_path, dent->d_name);
		if (stat(file_path, &file_stat) == -1) {
			perror(file_path);
			continue;
		}
		/* 파일 종류 */
		printf("%s", get_type_str(dent->d_type));
		/* 권한  표시*/
		print_permissions(file_stat.st_mode);
		/* 링크수 표시*/
		printf("%ld ", file_stat.st_nlink);
		/* 소유자명 표시*/
		owner = getpwuid(file_stat.st_uid);
		printf("%s ", owner->pw_name);
		/* 그룹명 표시*/
		group = getgrgid(file_stat.st_gid);
		printf("%s ", group->gr_name);
		/* 크기*/
		printf("%5ld ", file_stat.st_size);

		// 마지막으로 파일을 읽거나 실행한 시간
		// printf("%s", ctime(&file_stat.st_atime));

		// 파일의 내용이 마지막으로 수정된 시간
		struct tm *timeinfo = localtime(&file_stat.st_mtime);
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
		printf("%s ", buffer);

		// inode의 내용을 수정한 시간(파일의 상태가 마지막으로 변경된 시간)
		// printf("%s", ctime(&file_stat.st_ctime));

		// 심볼릭 링크인 경우
		if (strcmp("l", get_type_str(dent->d_type)) == 0) {
			printf("%s ", dent->d_name);
			read_symbolic(file_path);		//심볼릭 링크 출력
		}
		// 일반적인 경우
		else {
			printf("%s\n", dent->d_name);
		}
	}
	closedir(dp);
}

