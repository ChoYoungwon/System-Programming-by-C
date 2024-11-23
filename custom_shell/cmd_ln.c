#include "custom_header.h"

//ln 명령 실행 함수
void cmd_ln(int argc, char **argv)
{
	char *old_absolute = malloc(BUFSIZ);
    char *new_absolute = malloc(BUFSIZ);

    if(old_absolute == NULL || new_absolute == NULL) {
            perror("malloc");
            return;
    }
	//하드링크인 경우
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
	// 심볼릭 링크인 경우
	else if(argc == 4 && (strcmp(argv[1], "-s") == 0)) {
		// 절대 경로로 변환
		convert_to_absolute(argv[2], old_absolute);
		convert_to_absolute(argv[3], new_absolute);

		// 유효성 검사
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
