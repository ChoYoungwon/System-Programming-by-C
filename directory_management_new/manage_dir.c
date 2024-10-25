/*
 6주차 과제
 학번 : 2020136122
 이름 : 조영원
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <regex.h>

#define MAX_CMD_SIZE    (128)
#define MAX_ARG		(4)

/* 매크로 함수 선언을 자동화 */
#define DECLARE_CMDFUNC(str)	void cmd_##str(int argc, char **argv);

/* 함수 선언 */
DECLARE_CMDFUNC(help);
DECLARE_CMDFUNC(mkdir);
DECLARE_CMDFUNC(rmdir);
DECLARE_CMDFUNC(rename);
DECLARE_CMDFUNC(cd);
DECLARE_CMDFUNC(ls);
DECLARE_CMDFUNC(ln);
DECLARE_CMDFUNC(rm);
DECLARE_CMDFUNC(chmod);
DECLARE_CMDFUNC(cat);
DECLARE_CMDFUNC(cp);

/*작업 기본 경로*/
const char *BASE_PATH = "/tmp/test";


void convert_to_absolute(const char *relative_path, char *absolute_path);
int is_valid_path(const char *path);

typedef void (*cmd_func_t)(int argc, char **argv);

/*명령어 구조체 정의*/
typedef struct  cmd_t {
	char		cmd_str[MAX_CMD_SIZE];
	cmd_func_t	cmd_func;
} cmd_t;

/* 명령어 목록 */
static cmd_t cmd_list[] = {
	{"help", cmd_help},
	{"mkdir", cmd_mkdir},
	{"rmdir", cmd_rmdir},
	{"rename", cmd_rename},
	{"cd", cmd_cd},
	{"ls", cmd_ls},
	{"ln", cmd_ln},
	{"rm", cmd_rm},
	{"chmod", cmd_chmod},
	{"cat", cmd_cat},
	{"cp", cmd_cp},
};

const int command_num = sizeof(cmd_list) / sizeof(cmd_t);

/* 명령어에 의해 실행되는  함수를 찾는 함수 */
int search_command(char *cmd)
{
	int i;
	for (i = 0; i< command_num; i++) {
		if (strcmp(cmd, cmd_list[i].cmd_str) == 0) {
			return i;
		}
	}
	return (-1);
}


int main(int argc, char **argv)
{
    char *current_dir;		// 현대 디렉토리 주소
    char *tok_str[MAX_ARG];	// 토큰을 저장하는 포인터 배열
    int cmd_argc, i;
    char *appearance_dir;

    // 프롬프트 입력값 저장
    char *command;
    command = (char*) malloc(MAX_CMD_SIZE);
    if (command == NULL) {
        perror("malloc");
        exit(1);
    }

    /* /tmp/test 디렉토리가 없는 경우 */
    if (chdir(BASE_PATH) == -1)
    {
	mkdir(BASE_PATH, 0755);
	chdir(BASE_PATH);
    }
    do {
	current_dir = getcwd(NULL, BUFSIZ);	// 현재 디렉토리 주소

	//현재 주소가 홈 디렉토리인 경우
	if (strcmp(current_dir, BASE_PATH)==0) {
		printf("/ $ ");
	}
	/* 현재 주소가 다를 경우 */
	else {
		appearance_dir = malloc(strlen(current_dir) + strlen(BASE_PATH) + 1);
		strcpy(appearance_dir, current_dir + strlen(BASE_PATH));
		printf("%s $ ", appearance_dir);
		free(appearance_dir);
	}

	// 커맨드 창 입력을 받고  command 포인터 변수로 참조함
        if (fgets(command, MAX_CMD_SIZE-1, stdin) == NULL) break;

	// 첫번째 단어를 0번지 포인터 변수로 참조
        tok_str[0] = strtok(command, " \n");
        if (tok_str[0] == NULL) continue;

	for (cmd_argc = 1; cmd_argc < MAX_ARG; cmd_argc++)
	{
		tok_str[cmd_argc] = strtok(NULL, " \n");
		if (tok_str[cmd_argc] == NULL) {
			break;
		}
	}

	// 사용자가 quit 입력시 종료
	if (strcmp(tok_str[0], "quit") == 0) {
		break;
        } else {
		printf("your command: %s\n", tok_str[0]);

		i = search_command(tok_str[0]);
		if (i < 0) {
			printf("%s : command not found\n", tok_str[0]);
		} else {
			if (cmd_list[i].cmd_func) {
				cmd_list[i].cmd_func(cmd_argc, tok_str);
			}
			else {
				printf("no command function\n");
			}
		}
	    }
    } while (1);

    free(command);

    return 0;
}

/* help 명렁 실행 함수*/
void cmd_help(int argc, char **argv)
{
	printf("cd <path> : move to the specified directory\n");
	printf("mkdir <path> : cre시ate a directory at the specified path\n");
	printf("rmdir <path> : remove the directory at the specified path\n");
	printf("rename <source> <target> : rename the source directory to the target name\n");
	printf("ls : list contents and information of the current directory\n");
	printf("ln <original file> <new file> : create a hard link using link() to the target file with the specified link name\n");
        printf("rm <path> : remove a file using unlink() at the specified path\n");
    	printf("cat <filename> : display the contents of the specified file on the standard output\n");
    	printf("cp <original file> <new file> : copy the contents of the original file to the new file\n");
}

/* mkdir 명령 실행 함수 */
void cmd_mkdir(int argc, char **argv)
{
	char *absolute = malloc(BUFSIZ);
	if(absolute == NULL) {
		perror("malloc");
		return;
	}
	if (argc == 2) {
		convert_to_absolute(argv[1], absolute);	// 절대 경로 변환
		// 유효성 검사
		if(is_valid_path(absolute))
		{
			if(mkdir(absolute, 0755) == -1) {
				perror(absolute);
			}
		}
		else
			printf("You can't access upper directory\n");
	}
	else {
		printf("usage : mkdir <directory>\n");
	}
	free(absolute);
}

/* rmdir 명령 실행 함수 */
void cmd_rmdir(int argc, char **argv)
{
	char *absolute = malloc(BUFSIZ);
        if(absolute == NULL) {
                perror("malloc");
                return;
        }
	if (argc == 2 ){
		// 절대 경로로 변환
		convert_to_absolute(argv[1], absolute);

		// 유효성 검사
		if(is_valid_path(absolute))
		{
			if(rmdir(absolute) == -1) {
				perror(absolute);
			}
		}
		else
			printf("You can't access upper directory\n");
	}
	else {
		printf("usage : rmdir <directory>\n");
	}
	free(absolute);
}

/* rename 명령 실행 함수 */
void cmd_rename(int argc, char **argv)
{
	char *old_absolute = malloc(BUFSIZ);
	char *new_absolute = malloc(BUFSIZ);

        if(old_absolute == NULL || new_absolute == NULL) {
                perror("malloc");
                return;
        }
	if (argc == 3) {
		// 절대 경로로 변환 
		convert_to_absolute(argv[1], old_absolute);
		convert_to_absolute(argv[2], new_absolute);

		// 유효성 검사
		if(is_valid_path(old_absolute) && is_valid_path(new_absolute))
		{
			if(rename(old_absolute, new_absolute) == -1) {
				perror("rename");
			}
		}
		else
			printf("You can't access upper directory\n");
		}
	else {
		printf("usage : rmdir <old_name> <new_name>\n");
	}
	free(old_absolute);
	free(new_absolute);
}

/* cd 명령 실행 함수 */
void cmd_cd(int argc, char **argv)
{
	// 'cd ~' 명령시 홈 디렉토리로 이동
	if (argc == 1)
	{
		chdir(BASE_PATH);
	}
	else if (argc == 2 && strcmp(argv[1],"~") == 0) {
		chdir(BASE_PATH);
	}
	else {
		char *absolute = malloc(BUFSIZ);
	        if(absolute == NULL) {
	                perror("malloc");
	                return;
	        }
		if (argc == 2) {
			// 절대 경로로 변환
			convert_to_absolute(argv[1], absolute);

			// 유효성 검사
			if (is_valid_path(absolute)) {
				if(chdir(absolute) == -1) {
					perror("cd");
				}
			}
			else {
				printf("You can't access upper directory\n");
			}
		}
		else {
			printf("usage : cd <directory>\n");
		}
		free(absolute);
	}
}
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

/* ls 명령 실행 함수 */
void cmd_ls(int argc, char **argv)
{
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
		/* 마지막으로 파일을 읽거나 실행한 시간 */
		// printf("%s", ctime(&file_stat.st_atime));
		/* 파일의 내용이 마지막으로 수정된 시간*/
		struct tm *timeinfo = localtime(&file_stat.st_mtime);
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
		printf("%s ", buffer);
		/* inode의 내용을 수정한 시간(파일의 상태가 마지막으로 변경된 시간)*/
		// printf("%s", ctime(&file_stat.st_ctime));
		/* 이름 표시*/
		if (strcmp("l", get_type_str(dent->d_type)) == 0) {
			printf("%s ", dent->d_name);
			read_symbolic(file_path);
		}
		else {
			printf("%s\n", dent->d_name);
		}
	}
	closedir(dp);
}

/*ln 명령 실행 함수*/
void cmd_ln(int argc, char **argv)
{
	char *old_absolute = malloc(BUFSIZ);
        char *new_absolute = malloc(BUFSIZ);

        if(old_absolute == NULL || new_absolute == NULL) {
                perror("malloc");
                return;
        }
	/*하드링크인 경우*/
        if (argc == 3) {
                // 절대 경로로 변환
                convert_to_absolute(argv[1], old_absolute);
                convert_to_absolute(argv[2], new_absolute);

                // 유효성 검사
                if(is_valid_path(old_absolute) && is_valid_path(new_absolute))
                {
                        if(link(old_absolute, new_absolute) == -1) {
                                perror("link");
                        }
                }
                else
                        printf("You can't access upper directory\n");
        }
	/* 심볼릭 링크인 경우 */
	else if(argc == 4 && (strcmp(argv[1], "-s") == 0)) {
		/* 절대 경로로 변환 */
		convert_to_absolute(argv[2], old_absolute);
		convert_to_absolute(argv[3], new_absolute);

		/* 유효성 검사 */
		if(is_valid_path(old_absolute) && is_valid_path(new_absolute)) {
			if(symlink(old_absolute, new_absolute) == -1) {
				perror("symlink");
			}
		}
		else {
			printf("You can't access upper directory\n");
		}
	}
        else {
                printf("usage(hard link) : ln <old_name> <new_name>\n");
		printf("usage(symbolic link) : ln -s <target file> <original file>\n");
        }
        free(old_absolute);
        free(new_absolute);
}

/* 파일 삭제 명령(rm) */
void cmd_rm(int argc, char **argv)
{
	char *absolute = malloc(BUFSIZ);
        if(absolute == NULL) {
                perror("malloc");
                return;
        }
        if (argc == 2){
                // 절대 경로로 변환
                convert_to_absolute(argv[1], absolute);

                // 유효성 검사
                if(is_valid_path(absolute))
                {
                        if(unlink(absolute) == -1) {
                                perror(absolute);
                        }
                }
                else
                        printf("You can't access upper directory\n");
        }
        else {
                printf("usage : rm <directory>\n");
        }
        free(absolute);
}
mode_t chmod_format(const char *mode_str, mode_t current_mode)
{
	regex_t regex;			// 정규표현식 사용을 위한 변수 할당
	int result, i = 0;
	mode_t permission = 0;
	char op;
	mode_t mode = current_mode;
	//  정규표현식 컴파일
	if (regcomp(&regex, "^[0-7]+$", REG_EXTENDED) != 0) {
		return 0;
	}
	// 8진수 문자열인 경우 0을 반환(매칭 성공)
	//  일반 문자열인 경우 1을 반환(매칭 실패)
	result = regexec(&regex, mode_str, 0, NULL, 0);
	regfree(&regex);

	if (result == 0) {
		return (mode_t) strtol(mode_str, NULL, 8);
	}

	while(mode_str[i] != '\0' && (mode_str[i] == 'u' || mode_str[i] == 'g' || mode_str[i] == 'o')) {
		switch(mode_str[i]) {
			case 'u': permission |= S_IRUSR | S_IWUSR | S_IXUSR; break;
			case 'g': permission |= S_IRGRP | S_IWGRP | S_IXGRP; break;
			case 'o': permission |= S_IROTH | S_IWOTH | S_IXOTH; break;
			default: return (mode_t)-1;
		}
		i++;
	}
	if (permission == 0) {
		permission = 0777;
	}
	op = mode_str[i++];
	if (op != '+' && op != '-') {
		return (mode_t)-1;
	}
	while (mode_str[i] != '\0') {
		if(mode_str[i] == 'r' || mode_str[i] == 'w' || mode_str[i] == 'x') {
			switch(mode_str[i]) {
				case 'r':
					if (op == '+') mode |= permission & (S_IRUSR | S_IRGRP | S_IROTH);
					if (op == '-') mode &= ~(permission & (S_IRUSR | S_IRGRP | S_IROTH));
					break;
				case 'w':
                                        if (op == '+') mode |= permission & (S_IWUSR | S_IWGRP | S_IWOTH);
                                        if (op == '-') mode &= ~(permission & (S_IWUSR | S_IWGRP | S_IWOTH));
                                        break;
   				case 'x':
                                        if (op == '+') mode |= permission & (S_IXUSR | S_IXGRP | S_IXOTH);
                                        if (op == '-') mode &= ~(permission & (S_IXUSR | S_IXGRP | S_IXOTH));
                                        break;
				default:
					return -1;
			}
			i++;
		} else {
			return (mode_t)-1;
		}
	}
	return mode;
}

/*chmod 명령 함수*/
void cmd_chmod(int argc, char **argv)
{
	struct stat file_stat;
	char *absolute = malloc(BUFSIZ);
	if (absolute == NULL) {
		perror("malloc");
		return;
	}

	if (argc == 3) {
		convert_to_absolute(argv[2], absolute);
		if (stat(absolute, &file_stat) == -1) {
			perror("stat");
			free(absolute);
			return;
		}

		mode_t mode = chmod_format(argv[1], file_stat.st_mode);	// 문자열을 8진수 접근권환으로
		if (mode == (mode_t)-1) {
			printf("Invalid mode format\n");
			free(absolute);
			return;
		}
		if(is_valid_path(absolute)) {
			if(chmod(absolute, mode) == -1) {
				perror(absolute);
			}
		}
		else {
			printf("You can't access upper directory\n");
		}
	}
	else {
		printf("usage : chmod <접근권한: 8진수><파일명>");
	}
	free(absolute);
}

void cmd_cat (int argc, char **argv)
{
	FILE *file;
	char ch;
	char *absolute = malloc(BUFSIZ);
	if (argc == 2) {
		convert_to_absolute(argv[1], absolute);
		if (is_valid_path(absolute)) {
			file = fopen(argv[1], "r");
			if (file == NULL) {
				perror("file");
				free(absolute);
				return;
			}
			while((ch = fgetc(file)) != EOF) {
				putchar(ch);
			}
			fclose(file);
		}
		else {
			printf("You can't access upper directory\n");
		}
	}
	else {
		printf("usage: cat <파일명>\n");
	}
	free(absolute);
}
void copy_file(const char *source, const char *target)
{
	FILE *src = fopen(source, "rb");
	if (src == NULL) {
		perror("Error opening source file");
		fclose(src);
		return;
	}
	FILE *tar = fopen(target, "wb");
	if (target == NULL) {
		perror("Error opening source file");
		fclose(src);
		fclose(tar);
		return;
	}
	char buffer[BUFSIZ];
	size_t byte;

	while((byte = fread(buffer, 1, sizeof(buffer), src)) > 0) {
		fwrite(buffer, 1, byte, tar);
	}
	fclose(src);
	fclose(tar);
}

void cmd_cp (int argc, char **argv)
{
	char *absolute_origin = malloc(BUFSIZ);
	char *absolute_new = malloc(BUFSIZ);

	if (argc == 3) {
		convert_to_absolute(argv[1], absolute_origin);
		convert_to_absolute(argv[2], absolute_new);
		if (is_valid_path(absolute_origin) && is_valid_path(absolute_new)) {
			copy_file(absolute_origin, absolute_new);
		}
		else {
			printf("You can't access upper directory\n");
		}
	}
	else {
		printf("usage: cp <original file> <new file>\n");
	}
}

/* 상대 경로를 절대 경로로 변환하는 함수 */
void convert_to_absolute(const char *relative_path, char *absolute_path)
{
	char * current_path;
	current_path = getcwd(NULL, BUFSIZ);
	char *entire_path = malloc((strlen(relative_path)+strlen(current_path)+3));
	if(entire_path == NULL) {
	        perror("malloc");
	        return;
	}

	// absolute_path를 빈 문자열로 초기화한다.
	strcpy(absolute_path, "");

	// 상대 경로로 접근한 경우
	if (*relative_path != '/')
	{
		strcpy(entire_path, current_path);
		strcat(entire_path, "/");
		strcat(entire_path, relative_path);
	}
	// 절대 경로로 접근한 경우
	else {
		strcpy(entire_path, BASE_PATH);
		strcat(entire_path, relative_path);
	}
	// 함친 문자열을 절대 경로로 변환한다.
	char *token = strtok(entire_path, "/");
	while (token != NULL) {
	        if (strcmp(token, "..") == 0) {
	            // absolute_path 문자열의 마지막 / 이후의 경로를 지운다
	            char *slash = strrchr(absolute_path, '/');
	            if (slash != NULL && slash != absolute_path) {
	                *slash = '\0';
	            }
	        } else if (strcmp(token, "") != 0 && strcmp(token, ".") != 0) {
		    // absolute_path에 /'token'을 추가한다
	            strcat(absolute_path, "/");
	            strcat(absolute_path, token);
	        }
	        token = strtok(NULL, "/");
	}
	free(entire_path);
	// printf("implement_absolute_path : %s\n", absolute_path);
}

/* 유효성 검사 함수 (경로가 상위 디렉토리에 접근하는지 확인) */
int is_valid_path(const char *path)
{
	return (strncmp(path, BASE_PATH, strlen(BASE_PATH)) == 0);
}
