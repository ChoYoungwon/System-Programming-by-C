/*
4주차 과제
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

#define MAX_CMD_SIZE    (128)
#define BASE_PATH "/tmp/test"

void help();
void create_directory(const char *dir_name, const char *current_path);
void delete_directory(const char *dir_name, const char *current_path);
void rename_directory(const char *old_path, const char *new_path, const char *current_path);
void move_directory(const char *path, const char *current_path);
void convert_to_absolute(const char *relative_path, char *absolute_path, const char *current);
int is_valid_path(const char *path);

int main(int argc, char **argv)
{
    // 현대 디렉토리 주소
    char *current_dir;

    // 보여지는 주소
    char *appearance_dir;

    // ls 명렁에서 사용
    DIR *dp;
    struct dirent *dent;
    struct stat file_stat;
    struct passwd *owner;
    struct group *group;
    char *file_path = (char *)malloc(100 * sizeof(char));
    if (file_path == NULL) {
	perror("malloc");
	exit(1);
    }

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
	char *tok_str[3] = {NULL, NULL, NULL};	// 토큰을 저장하는 포인터 배열
	current_dir = getcwd(NULL, BUFSIZ);	// 현재 디렉토리 주소

	//현재 주소가 홈 디렉토리인 경우
	if (strcmp(current_dir, BASE_PATH)==0)
		printf("/ $ ");
	// 현재 주소가 다를 경우
	else
	{
		appearance_dir = malloc(strlen(current_dir) + strlen(BASE_PATH) + 1);
		strcpy(appearance_dir, current_dir + strlen(BASE_PATH));
		printf("%s $ ", appearance_dir);
		free(appearance_dir);
	}
	// 현재 디렉토리의 하위 폴더를 dp 구조체 변수로 저장
	dp = opendir(current_dir);

	// 커맨드 창 입력을 받고  command 포인터 변수로 참조함
        if (fgets(command, MAX_CMD_SIZE-1, stdin) == NULL) break;

	// 첫번째 단어를 0번지 포인터 변수로 참조
        tok_str[0] = strtok(command, " \n");
        if (tok_str[0] == NULL) continue;

	// 사용자가 quit 입력시 종료
        if (strcmp(tok_str[0], "quit") == 0) {
            break;
        } else {
            printf("your command: %s\n", tok_str[0]);

	    // help 명렁 수행
	    if (strcmp(tok_str[0], "help") == 0)
	    {
		help();
		continue;
	    }
	    // mkdir 명령 수행
	    if (strcmp(tok_str[0], "mkdir") == 0 )
	    {
		tok_str[1] = strtok(NULL, " \n");
		create_directory(tok_str[1], current_dir);
            	continue;
	    }
	    // rmdir 명령 수행
	    if (strcmp(tok_str[0], "rmdir") == 0 )
	    {
		tok_str[1] = strtok(NULL, " \n");
		delete_directory(tok_str[1], current_dir);
		continue;
            }
	    // rename 명령 수행
	    if (strcmp(tok_str[0], "rename") == 0 )
	    {
		tok_str[1] = strtok(NULL, " \n");
		tok_str[2] = strtok(NULL, " \n");
		rename_directory(tok_str[1], tok_str[2], current_dir);
		continue;
	    }
	    // cd 명령 수행
	    if (strcmp(tok_str[0], "cd") == 0 )
	    {
		tok_str[1] = strtok(NULL, " \n");
		move_directory(tok_str[1] ? tok_str[1] : "~", current_dir);
	    }
	    // ls 명령 수행
	    if (strcmp(tok_str[0], "ls") == 0 )
	    {
		while((dent = readdir(dp))) {
			strcpy(file_path, current_dir);
			strcat(file_path, "/");
			strcat(file_path, dent->d_name);
			if (stat(file_path, &file_stat) == -1) {
				perror(file_path);
				continue;
			}
			// 권한 8진수로 표시 
			printf("%o ", file_stat.st_mode);
			// 링크수 표시
			printf("%ld ", file_stat.st_nlink);
			// 소유자명 표시
			owner = getpwuid(file_stat.st_uid);
			printf("%s ", owner->pw_name);
			// 그룹명 표시
			group = getgrgid(file_stat.st_gid);
			printf("%s ", group->gr_name);
			// 크기
			printf("%5ld ", file_stat.st_size);
			// 이름 표시
			printf("%s\n", dent->d_name);
		}
	    }
	    closedir(dp);
	}
    } while (1);

    free(command);
    free(file_path);

    return 0;
}

// help 명렁 실행 함수
void help()
{
	printf("cd <path> : move to the specified directory\n");
	printf("mkdir <path> : cre시ate a directory at the specified path\n");
	printf("rmdir <path> : remove the directory at the specified path\n");
	printf("rename <source> <target> : rename the source directory to the target name\n");
	printf("ls : list contents and information of the current directory\n");
}

// mkdir 명령 실행 함수
void create_directory(const char *dir_name, const char *current_path)
{
	char *absolute = malloc(BUFSIZ);
	if(absolute == NULL) {
		perror("malloc");
		return;
	}

	// 절대 경로 변환
	convert_to_absolute(dir_name, absolute, current_path);

	// 유효성 검사
	if(is_valid_path(absolute))
	{
		if(mkdir(absolute, 0755) == -1) {
			perror(absolute);
		}
	}
	else
		printf("You can't access upper directory\n");

	free(absolute);
}

// rmdir 명령 실행 함수
void delete_directory(const char *dir_name, const char *current_path)
{
	char *absolute = malloc(BUFSIZ);
        if(absolute == NULL) {
                perror("malloc");
                return;
        }

	// 절대 경로로 변환
	convert_to_absolute(dir_name, absolute, current_path);

	// 유효성 검사
	if(is_valid_path(absolute))
	{
		if(rmdir(absolute) == -1) {
			perror(absolute);
		}
	}
	else
		printf("You can't access upper directory\n");

	free(absolute);
}

// rename 명령 실행 함수
void rename_directory(const char *old_path, const char *new_path, const char *current_path)
{
	char *old_absolute = malloc(BUFSIZ);
	char *new_absolute = malloc(BUFSIZ);

        if(old_absolute == NULL || new_absolute == NULL) {
                perror("malloc");
                return;
        }

	// 절대 경로로 변환 
	convert_to_absolute(old_path, old_absolute, current_path);
	convert_to_absolute(new_path, new_absolute, current_path);

	// 유효성 검사
	if(is_valid_path(old_absolute) && is_valid_path(new_absolute))
	{
		if(rename(old_absolute, new_absolute) == -1) {
			perror("rename");
		}
	}
	else
		printf("You can't access upper directory\n");

	free(old_absolute);
	free(new_absolute);
}

// cd 명령 실행 함수
void move_directory(const char *path, const char *current_path)
{
	// 'cd ~' 명령시 홈 디렉토리로 이동
	if (strcmp(path, "~")==0)
	{
		chdir(BASE_PATH);
	}
	else {
		char *absolute = malloc(BUFSIZ);
	        if(absolute == NULL) {
	                perror("malloc");
	                return;
	        }
		// 절대 경로로 변환
		convert_to_absolute(path, absolute, current_path);

		// 유효성 검사
		if (is_valid_path(absolute))
		{
			if(chdir(absolute) == -1) {
				perror("cd");
			}
		}
		else
		{
			printf("You can't access upper directory\n");
		}
		free(absolute);
	}
}

// 상대 경로를 절대 경로로 변환하는 함수 
void convert_to_absolute(const char *relative_path, char *absolute_path, const char *current_path)
{
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

// 유효성 검사 함수 (경로가 상위 디렉토리에 접근하는지 확인)
int is_valid_path(const char *path)
{
	return (strncmp(path, BASE_PATH, strlen(BASE_PATH)) == 0);
}
