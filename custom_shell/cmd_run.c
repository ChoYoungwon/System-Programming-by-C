#include "config.h"
#ifdef ENABLE_CMD_RUN
#include "custom_header.h"

void cmd_run(int argc, char **argv)
{
	(void)argc;
	pid_t pid;
	char *absolute = malloc(BUFSIZ);
	if (absolute == NULL) {
		perror("malloc");
		return;
	}
	convert_to_absolute(argv[0], absolute);

	if(is_valid_path(absolute) == 0){
		printf("You can't access upper directory\n");
		free(absolute);
		return;
	} else {
		switch (pid = fork()) {
			case -1:	// fork 실패한 경우
				perror("fork");
				exit(1);
				break;
			case 0:		// 자식 프로세스(프로그램 실행)
				if (execve(absolute, argv, NULL) == -1) {
					perror("execve");
					exit(1);
				}
				free(absolute);
				exit(0);
				break;
			default:	// 부모 프로세스
				if(waitid(P_PID, pid, NULL, WEXITED) == -1) {
					perror("waitid");
				}
				free(absolute);
				break;
		}
	}
}

#endif
