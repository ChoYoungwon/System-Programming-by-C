#include "config.h"
#ifdef ENABLE_CMD_LS
#include "custom_header.h"

// 파일 종류 반환 함수
static const char *get_type_str(char type) {
    switch (type) {
        case DT_BLK: return "b";
        case DT_CHR: return "c";
        case DT_DIR: return "d";
        case DT_FIFO: return "p";
        case DT_LNK: return "l";
        case DT_REG: return "-";
        case DT_SOCK: return "s";
        default: return "u";
    }
}

// 파일의 접근 권한을 rwx 형태로 출력하는 함수
static void print_permissions(mode_t mode) {
    char perms[10];
    perms[0] = (mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (mode & S_IXUSR) ? ((mode & S_ISUID) ? 's' : 'x') : ((mode & S_ISUID) ? 'S' : '-');
    perms[3] = (mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (mode & S_IXGRP) ? ((mode & S_ISGID) ? 's' : 'x') : ((mode & S_ISGID) ? 'S' : '-');
    perms[6] = (mode & S_IROTH) ? 'r' : '-';
    perms[7] = (mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (mode & S_IXOTH) ? ((mode & S_ISVTX) ? 't' : 'x') : ((mode & S_ISVTX) ? 'T' : '-');
    perms[9] = '\0';
    printf("%s ", perms);
}

/*심벌릭 링크 내용 읽고 출력하는  함수*/
static void read_symbolic(char *path)
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


// 파일/디렉토리 정보 출력
static void run_li(const char *path, int run_li, int run_a) {
	char dir_path[BUFSIZ];

    // 절대경로로 변환
    convert_to_absolute(path, dir_path);

    if (!is_valid_path(dir_path)) {
        fprintf(stderr, "Error: Invalid path: %s\n", dir_path);
        return;
    }

    DIR *dp = opendir(dir_path);
    if (!dp) {
        perror("opendir");
        return;
    }

    struct dirent *dent;
    struct stat file_stat;
    int i = 0;

    while ((dent = readdir(dp))) {
		// printf("Debug: Reading file = %s\n", dent->d_name);
		// printf("Debug: a : %d\n", run_a);

        // 숨김 파일 제외 처리
		if (run_a == 0) {
	        if (dent->d_name[0] == '.') {
	            continue;
	        }
		}

        // 파일 경로 생성
		size_t path_length = strlen(dir_path)+strlen(dent->d_name) +2;
		char *file_path = malloc(path_length);
		if (file_path == NULL) {
			perror("malloc");
			continue;
		}

        snprintf(file_path, path_length, "%s/%s", dir_path, dent->d_name);

        if (run_li) {
            if (lstat(file_path, &file_stat) == -1) {
                perror(file_path);
                continue;
            }

            // 파일 종류
            printf("%s", get_type_str(dent->d_type));
            // 권한
			print_permissions(file_stat.st_mode);
            // 링크수
			printf("%ld ", file_stat.st_nlink);
			// 소유자, 그룹명 표시
            struct passwd *owner = getpwuid(file_stat.st_uid);
            struct group *group = getgrgid(file_stat.st_gid);
            printf("%s %s ", owner ? owner->pw_name : "unknown", group ? group->gr_name : "unknown");

			//파일 크기
            printf("%5ld ", file_stat.st_size);

			// 파일의 내용이 마지막으로 수정된 시간
            char time_buf[64];
            struct tm *timeinfo = localtime(&file_stat.st_mtime);
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", timeinfo);
            printf("%s ", time_buf);

			// 심볼릭 링크인 경우
            if (strcmp("l", get_type_str(dent->d_type))== 0) {
                printf("%s ", dent->d_name);
                read_symbolic(file_path);
            } else {
                printf("%s\n", dent->d_name);
            }
        } else {
            printf("%s  ", dent->d_name);
            i++;
            if (i % 6 == 0) {
                printf("\n");
            }
        }
    }
    if ((i != 0) && (i != 6)) {
		printf("\n");
	}
    closedir(dp);
}

// ls 명령 실행
void cmd_ls(int argc, char **argv) {
    int n = 0, all = 0, list = 0;

    // getopt 상태 초기화
    optind = 0;

	/*
	printf("Debug: initial optind = %d\n", optind);
	for (int i =0; i < argc; i++)
	{
		printf("Debug: %s ", argv[i]);
	}
	printf("\n");
    */
	while ((n = getopt(argc, argv, "al")) != -1) {
		// printf("Debug: getopt returned %c, optind = %d, optarg = %s\n", n, optind, optarg ? optarg : "(null)");
        switch (n) {
            case 'a':
                all = 1;
                break;
            case 'l':
                list = 1;
                break;
            case '?':
                printf("usage: ls [-a] [-l] <path>\n");
                return;
        }
    }

	// printf("Debug: final optind = %d, a = %d, l = %d\n", optind, all, list);

    if (optind >= argc) {
        run_li(".", list, all); // 현재 디렉토리 출력
    } else {
        for (int i = optind; i < argc; i++) {
            run_li(argv[i], list, all); // 인자로 받은 경로 출력
        }
    }

    // 상태 초기화
    optind = 0;
}

#endif
