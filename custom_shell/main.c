#include "custom_header.h"

typedef void (*cmd_func_t)(int argc, char **argv);

//명령어 구조체 정의
typedef struct  cmd_t {
	char		cmd_str[MAX_CMD_SIZE];
	cmd_func_t	cmd_func;
} cmd_t;

// 명령어 목록
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
	{"ps", cmd_ps},
	{"kill", cmd_kill},
};

// 명령어 개수
const int command_num = sizeof(cmd_list) / sizeof(cmd_t);

// 명령어에 의해 실행되는 함수를 찾는 함수
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
	(void)argc;
	(void)argv;

	// Ctrl+C 입력 방지
    void (*hand)(int);
    hand = signal(SIGINT, SIG_IGN);
    if (hand == SIG_ERR) {
    	perror("signal");
	exit(1);
    }

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

    // /tmp/test 디렉토리가 없는 경우
    if (chdir(BASE_PATH) == -1)
    {
		mkdir(BASE_PATH, 0755);
		if (chdir(BASE_PATH) != 0) {
			perror("chdir");
			exit(1);
		}
    }
    do {
		optind = 1;
		opterr = 0;
		optopt = 0;
		current_dir = getcwd(NULL, BUFSIZ);	// 현재 디렉토리 주소

		//현재 주소가 홈 디렉토리인 경우
		if (strcmp(current_dir, BASE_PATH)==0) {
			printf("/ $ ");
		}
		// 현재 주소가 홈 디렉토리와  다를 경우
		else {
			appearance_dir = malloc(strlen(current_dir) + strlen(BASE_PATH) + 1);
			strcpy(appearance_dir, current_dir + strlen(BASE_PATH));
			printf("%s $ ", appearance_dir);
			free(appearance_dir);
		}

		// 커맨드 창 입력을 받고, command 포인터 변수 참조
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
				if(cmd_argc == 1) {
					cmd_run(cmd_argc, tok_str);					// 프로그램 실행
				} else {
					printf("%s : command not found\n", tok_str[0]);
				}
			} else {
				if (cmd_list[i].cmd_func) {
					cmd_list[i].cmd_func(cmd_argc, tok_str);	// 명령어 실행
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
