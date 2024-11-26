#include "config.h"
#ifdef ENABLE_CMD_LN
#include "custom_header.h"

//ln 명령 실행 함수
void cmd_ln(int argc, char **argv)
{
	int n;
	int symbolic_link = 0;
	optind = 0;

	while ((n = getopt(argc, argv, "s")) != -1) {
		switch (n) {
		case 's':
			symbolic_link = 1;
			break;
		}
	}

	if (argc - optind < 2) {
		printf("usage: ln [-s] <target file> <link file>\n");
		return;
	}

	char *old_absolute = malloc(BUFSIZ);
    char *new_absolute = malloc(BUFSIZ);

    if(old_absolute == NULL || new_absolute == NULL) {
            perror("malloc");
            return;
    }

	// 절대 경로로 변환
	convert_to_absolute(argv[optind], old_absolute);
	convert_to_absolute(argv[optind + 1], new_absolute);


	if (!symbolic_link) {	// 하드 링크인 경우

        // 유효성 검사
        if(is_valid_path(old_absolute) && is_valid_path(new_absolute)) {
            if(link(old_absolute, new_absolute) == -1) {
                perror("link");
            }
        } else {
            printf("You can't access upper directory\n");
		}
	} else {	// 심볼릭 링크인 경우

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
        free(old_absolute);
        free(new_absolute);
}

#endif
