#include "custom_header.h"

// 파일 종류 반환 함수
const char *get_type_str(char type) {
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
void print_permissions(mode_t mode) {
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

// 심볼릭 링크 내용 읽고 출력하는 함수
void read_symbolic(const char *path) {
    char buf[BUFSIZ];
    int n = readlink(path, buf, BUFSIZ);
    if (n == -1) {
        perror("readlink");
        return;
    }
    buf[n] = '\0';
    printf("-> %s\n", buf);
}

// 파일/디렉토리 정보 출력
void run_li(const char *path, int l, int a) {
    char file_path[BUFSIZ]; // 동적 메모리 대신 스택 메모리 사용

    // 절대경로로 변환
    convert_to_absolute(path, file_path);

    if (!is_valid_path(file_path)) {
        fprintf(stderr, "Error: Invalid path: %s\n", file_path);
        return;
    }

    DIR *dp = opendir(file_path);
    if (!dp) {
        perror("opendir");
        return;
    }

    struct dirent *dent;
    struct stat file_stat;
    int i = 0;

    while ((dent = readdir(dp))) {
        // 숨김 파일 제외 처리
        if (!a && dent->d_name[0] == '.') {
            continue;
        }

        // 파일 경로 생성
        snprintf(file_path, BUFSIZ, "%s/%s", path, dent->d_name);

        if (l) {
            if (stat(file_path, &file_stat) == -1) {
                perror(file_path);
                continue;
            }

            // 파일 정보 출력
            printf("%s", get_type_str(dent->d_type));
            print_permissions(file_stat.st_mode);
            printf("%ld ", file_stat.st_nlink);

            struct passwd *owner = getpwuid(file_stat.st_uid);
            struct group *group = getgrgid(file_stat.st_gid);
            printf("%s %s ", owner ? owner->pw_name : "unknown", group ? group->gr_name : "unknown");

            printf("%5ld ", file_stat.st_size);

            char time_buf[64];
            struct tm *timeinfo = localtime(&file_stat.st_mtime);
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", timeinfo);
            printf("%s ", time_buf);

            if (S_ISLNK(file_stat.st_mode)) {
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
	printf("\n");
    closedir(dp);
}

// ls 명령 실행 함수
void cmd_ls(int argc, char **argv) {
    int n, l = 0, a = 0;

	optind = 1;
    while ((n = getopt(argc, argv, "al")) != -1) {
        switch (n) {
            case 'a': a = 1; break;
            case 'l': l = 1; break;
            case '?':
                printf("usage: ls [-a] [-l] <path>\n");
                return;
        }
    }

    if (optind >= argc) {
        run_li(".", l, a); // 현재 디렉토리 출력
    } else {
        for (int i = optind; i < argc; i++) {
            run_li(argv[i], l, a); // 인자로 받은 경로 출력
        }
    }
}
