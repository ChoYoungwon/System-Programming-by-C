#include "custom_header.h"

// cd 명령 실행 함수
void cmd_cd(int argc, char **argv)
{
	// 'cd ~' 명령시 홈 디렉토리로 이동
	if (argc == 1) {
		if (chdir(BASE_PATH) != 0) {
			perror("chdir");
			return;
		}
	}
	else if (argc == 2 && strcmp(argv[1],"~") == 0) {
		if (chdir(BASE_PATH) != 0) {
			perror("chdir");
			return;
		}
	}
	else {
		char *absolute = malloc(BUFSIZ);
	        if(absolute == NULL) {
	                perror("malloc");
	                return;
	        }
		if (argc == 2) {
			convert_to_absolute(argv[1], absolute);		// 절대경로로 변환

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
